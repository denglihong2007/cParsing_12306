using Newtonsoft.Json.Linq;

class Parsing_12306
{
    public static Dictionary<string, string> stationTelPairs=[];
    public static Dictionary<string, string> trainNoPairs = [];
    public static Dictionary<string, string> trainNoStartEndPairs = [];
    public static List<string[]> data = [];
    public static string date;
    public static string station;
    public static async Task<Dictionary<string, string>> GetStationTelPairs()
    {
        Dictionary<string, string> stationTelPairs_ = [];
        HttpClient client = new();
        string url = "https://kyfw.12306.cn/otn/resources/js/framework/station_name.js?station_version=1.9278";
        HttpResponseMessage response = await client.GetAsync(url);
        if (response.IsSuccessStatusCode)
        {
            List<string> responseBody = [.. (await response.Content.ReadAsStringAsync()).Split("@")];
            responseBody.RemoveAt(0);
            foreach (string line in responseBody)
            {
                string[] objects = line.Split("|");
                stationTelPairs_.Add(objects[1], objects[2]);
            }
        }
        return stationTelPairs_;
    }
    #region cParsing_Train
    public static async Task GetTrains(int index_)
    {
        int max = 250 * (index_ + 1);
        if(max> stationTelPairs.Values.Count)
        {
            max= stationTelPairs.Values.Count;
        }
        Console.WriteLine(max);
        for (int i = 250 * index_; i < max; i++)
        {
            Console.WriteLine("Thread A "+index_+" Progress:" + 100*(i - 250 * index_)/ (max- 250 * index_) + "%");
            HttpClient client = new();
            client.DefaultRequestHeaders.Add("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36 Edg/120.0.0.0");
            string url = "https://kyfw.12306.cn/otn/leftTicketPrice/query?leftTicketDTO.train_date=" + date + "&leftTicketDTO.from_station=" + stationTelPairs[station] + "&leftTicketDTO.to_station=" + stationTelPairs.Values.ToArray()[i] + "&leftTicketDTO.ticket_type=1&randCode=stzh";
            HttpResponseMessage response = await client.GetAsync(url);
            if (response.IsSuccessStatusCode)
            {
                JObject jsonObject = JObject.Parse(await response.Content.ReadAsStringAsync());
                JArray dataArray = (JArray)jsonObject["data"];
                foreach (JToken item in dataArray)
                {
                    string trainNo = item["queryLeftNewDTO"]["train_no"].ToString();
                    if (!trainNoPairs.ContainsKey(trainNo))
                    {
                        string stationTrainCode = item["queryLeftNewDTO"]["station_train_code"].ToString();
                        string start_station_name = item["queryLeftNewDTO"]["start_station_name"].ToString();
                        string end_station_name = item["queryLeftNewDTO"]["end_station_name"].ToString();
                        trainNoPairs[stationTrainCode] = trainNo;
                        trainNoStartEndPairs[trainNo] = start_station_name+","+ end_station_name;
                    }
                }
            }
        }
        Console.WriteLine("Thread A " + index_ + " has finished");
    }
    public static async Task Parsing_Train() 
    {
        if (stationTelPairs.ContainsKey(station))
        {
            List<Task> tasks = [];
            for (int i = 0; i < 14; i++)
            {
                int currentIndex = i;
                Task task = Task.Run(() => GetTrains(currentIndex));
                tasks.Add(task);
            }
            await Task.WhenAll(tasks);
        }
    }
    #endregion
    #region cParsing_TimeTable
    public static async Task GetTimeTable(int index_)
    {
        int max = 50 * (index_ + 1);
        if (max > trainNoPairs.Values.Count)
        {
            max = trainNoPairs.Values.Count;
        }
        Console.WriteLine(max);
        for (int i = 50 * index_; i < max; i++)
        {
            Console.WriteLine("Thread B " + index_ + " Progress:" + 100 * (i - 50 * index_) / (max - 50 * index_) + "%");
            HttpClient client = new();
            client.DefaultRequestHeaders.Add("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36 Edg/120.0.0.0");
            string url = "https://kyfw.12306.cn/otn/queryTrainInfo/query?leftTicketDTO.train_no=" + trainNoPairs.Values.ToList()[i] + "&leftTicketDTO.train_date=" + date + "&rand_code=";
            HttpResponseMessage response = await client.GetAsync(url);
            if (response.IsSuccessStatusCode)
            {
                JObject jsonObject = JObject.Parse(await response.Content.ReadAsStringAsync());
                JArray dataArray = (JArray)jsonObject["data"]["data"];
                foreach (JObject item in dataArray.Cast<JObject>())
                {
                    string stationName = item["station_name"].ToString();
                    if (stationName == station)
                    {
                        string arriveTime = item["arrive_time"].ToString();
                        string startTime = item["start_time"].ToString();
                        string station_train_code = item["station_train_code"].ToString();
                        if (arriveTime == "----")
                        {
                            arriveTime = startTime;
                        }
                        if (startTime == "----")
                        {
                            startTime = arriveTime;
                        }
                        data.Add([station_train_code, arriveTime, startTime, trainNoStartEndPairs[trainNoPairs.Values.ToList()[i]]]);
                        break;
                    }
                }
            }
        }
        Console.WriteLine("Thread B " + index_ + " has finished");
    }
    public static async Task Parsing_TimeTable()
    {
        List<Task> tasks = [];
        int number=trainNoPairs.Count/50+1;
        for (int i = 0; i < number+1; i++)
        {
            int currentIndex = i;
            Task task = Task.Run(() => GetTimeTable(currentIndex));
            tasks.Add(task);
        }
        await Task.WhenAll(tasks);
    }
    #endregion

    public static async Task Main(string[] args)
    {
        try
        {
            stationTelPairs = GetStationTelPairs().Result;
#if DEBUG
            args = ["成都西", "2023-11-21"];
#endif
            date = args[1];
            station = args[0];
            await Parsing_Train();
            await Parsing_TimeTable();
        }
        catch (Exception e)
        {
            using StreamWriter sw = new(".\\error.txt");
            sw.WriteLine(string.Join(",", e.Message));
        }
        using (StreamWriter sw = new(".\\result.csv"))
        {
            foreach (string[] row in data)
            {
                sw.WriteLine(string.Join(",", row));
            }
        }
    }
}