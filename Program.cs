using Newtonsoft.Json.Linq;
using static System.Runtime.InteropServices.JavaScript.JSType;

class cParsing_12306
{
    public static Dictionary<string, string> stationTelPairs=[];
    public static Dictionary<string, string> trainNoPairs = [];
    public static List<string[]> data = [];
    public static string date;
    public static string station;
    public static async Task<Dictionary<string, string>> getStationTelPairs()
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
    public static async Task getTrains(int index_)
    {
        int max = 250 * (index_ + 1);
        if(max> stationTelPairs.Values.Count)
        {
            max= stationTelPairs.Values.Count;
        }
        Console.WriteLine(max);
        for (int i = 250 * index_; i < max; i++)
        {
            Console.WriteLine("Thread "+index_+" Progress:" + 100*(i - 250 * index_)/ (max- 250 * index_) + "%");
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
                        trainNoPairs[stationTrainCode] = trainNo;
                    }
                }
            }
        }
    }
    public static async Task cParsing_Train() 
    {
        if (stationTelPairs.ContainsKey(station))
        {
            List<Task> tasks = [];
            for (int i = 0; i < 14; i++)
            {
                int currentIndex = i;
                Task task = Task.Run(() => getTrains(currentIndex));
                tasks.Add(task);
            }
            await Task.WhenAll(tasks);
        }
    }
    #endregion
    #region cParsing_TimeTable
    public static async Task cParsing_TimeTable()
    {
        Parallel.ForEach(trainNoPairs.Values,async (item1) =>
        {
            HttpClient client = new();
            client.DefaultRequestHeaders.Add("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36 Edg/120.0.0.0");
            string url = "https://kyfw.12306.cn/otn/queryTrainInfo/query?leftTicketDTO.train_no="+item1+"&leftTicketDTO.train_date=" + date + "&rand_code=";
            HttpResponseMessage response = await client.GetAsync(url);
            if (response.IsSuccessStatusCode)
            {
                JObject jsonObject = JObject.Parse(await response.Content.ReadAsStringAsync());
                JArray dataArray = (JArray)jsonObject["data"]["data"];
                foreach (JObject item in dataArray)
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
                        data.Add([station_train_code, arriveTime, startTime]);
                        break;
                    }
                }
            }
        });
    }
    #endregion

    public static async Task Main(string[] args)
    {
        try
        {
            stationTelPairs = getStationTelPairs().Result;
#if DEBUG
            args = ["广元", "2023-11-21"];
#endif
            date = args[1];
            station = args[0];
            await cParsing_Train();
            await cParsing_TimeTable();
        }
        catch (Exception e)
        {
            Console.WriteLine(e.Message);
        }
        Console.WriteLine("Press any key to create CSV file.");
        Console.ReadLine();
        using (StreamWriter sw = new(".\\result.csv"))
        {
            foreach (string[] row in data)
            {
                sw.WriteLine(string.Join(",", row));
            }
        }

        Console.WriteLine("CSV file created successfully.");
    }
}