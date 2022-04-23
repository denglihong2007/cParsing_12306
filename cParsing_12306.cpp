#include<windows.h>
#include<string.h>
#include<iostream>
#include<string>
#include<fstream>
#include<stdlib.h>
#include<vector>
#include<fstream>
#include<codecvt>
#include<locale>
#pragma execution_character_set("utf-8")
#include"requests.h"
#include"cJSON.h"
using namespace std;
using namespace requests;

string Lpcwstr2String(LPCWSTR lps)
{
	int len = WideCharToMultiByte(CP_ACP, 0, lps, -1, NULL, 0, NULL, NULL);
	if (len <= 0)
	{
		return "";
	}
	else
	{
		char* dest = new char[len];
		WideCharToMultiByte(CP_ACP, 0, lps, -1, dest, len, NULL, NULL);
		dest[len - 1] = 0;
		string str(dest);
		delete[] dest;
		return str;
	}
}

std::string Utf8ToGbk(std::string src_str1) //const char *src_str
{
	const char* src_str = src_str1.data();
	int len = MultiByteToWideChar(CP_UTF8, 0, src_str, -1, NULL, 0);
	wchar_t* wszGBK = new wchar_t[len + 1];
	memset(wszGBK, 0, len * 2 + 2);
	MultiByteToWideChar(CP_UTF8, 0, src_str, -1, wszGBK, len);
	len = WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, NULL, 0, NULL, NULL);
	char* szGBK = new char[len + 1];
	memset(szGBK, 0, len + 1);
	WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, szGBK, len, NULL, NULL);
	std::string strTemp(szGBK);
	if (wszGBK) delete[] wszGBK;
	if (szGBK) delete[] szGBK;
	return strTemp;
}

string get_path()
{

	OPENFILENAME ofn;
	char szFile[300];

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = (LPWSTR)szFile;
	ofn.lpstrFile[0] = '\0';
	LPTSTR        lpstrCustomFilter;
	DWORD         nMaxCustFilter;
	ofn.nFilterIndex = 1;
	LPTSTR        lpstrFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = L"文本文档\0*.TXT\0";
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	string path_image = "";
	if (GetOpenFileName(&ofn))
	{
		path_image = Lpcwstr2String(ofn.lpstrFile);
		return path_image;
	}
	else
	{
		return "";
	}
}

bool AllisNum(string str)
{
	for (int i = 0; i < str.size(); i++)
	{
		int tmp = (int)str[i];
		if (tmp >= 48 && tmp <= 57)
		{
			continue;
		}
		else
		{
			return false;
		}
	}
	return true;
}

vector<string> split(string str, string pattern)
{
	string::size_type pos;
	vector<string> result;
	str += pattern;//扩展字符串以方便操作
	int size = str.size();
	for (int i = 0; i < size; i++)
	{
		pos = str.find(pattern, i);
		if (pos < size)
		{
			string s = str.substr(i, pos - i);
			result.push_back(s);
			i = pos + pattern.size() - 1;
		}
	}
	return result;
}

string get_search(string train_number, string date,map<string, string> header, map<string, string> options)
{
	string r;
	if (options["proxy"] != "None")
	{
		Response resp = Get("https://search.12306.cn/search/v1/train/search?callback=jQuery&keyword=" + train_number + "&date=" + date, header, "", options);
		r =  resp.GetText();
	}
	else
	{
		Response resp = Get("https://search.12306.cn/search/v1/train/search?callback=jQuery&keyword=" + train_number + "&date=" + date, header);
		r =  resp.GetText();
	}
	int index = r.find("网络");
	if (index < r.length())
	{
		cout << "网络可能存在问题。" << endl;
		system("pause");
		exit(0);
	}
	else
	{
		return (r.substr(11, r.size() - 42) + "}");
	}
}

ofstream csv("qETRC.csv");

int main()
{
	system("chcp 65001");
	string date;
	string train_info;
	string train_number;
	string search_result;
	string porxies;
	string path;
	vector<string> rule;
	char use_rules;
	char use_porxies;
	char txt[100];
	ifstream infile;
	vector<string> rules_list;
	
	map<string, string> options;
	map<string, string> header;
	map<string, string> search;
	map<string, string> train;
	header["User-Agent"] = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/99.0.4844.74 Safari/537.36 Edg/99.0.1150.52";

    cout << "欢迎使用cParsing_12306，本项目使用并遵循GPLv3协议，程序作者：denglihong2007。\n";
    cout << "请问您是否需要导入规则文件（UTF-8编码，格式为A B 1 0，意为将A站改为B站且把到达时间推迟一分钟设为通过状态）？（是的话输入Y，否则输入其它键）";
    cin >> use_rules;

    if ('Y' == use_rules || 'y' == use_rules)
	{
		infile.open(get_path());
		if (!infile.is_open())
		{
			cout << "打开失败。" << endl;
			system("pause");
			exit(0);
		}

		while (!infile.eof())
		{
			infile.getline(txt, 100);
			rules_list.push_back(txt);
		}
		infile.close();
		getchar();
	}
	else
	{
		rules_list.push_back("none");
	}

	cout << "请问您是否需要设置网络代理？（是的话输入Y，否则输入其它键）";
	cin >> use_porxies;

	if ('Y' == use_porxies || 'y' == use_porxies)
	{
		cout << "请输入代理http地址。";
		cin >> porxies;
		options["proxy"] = "https=" + porxies;
	}
	else
	{
		options["proxy"] = "None";
	}

	int num = 0;
	while (num < 2)
	{
		if (num == 0)
		{
			cout << "您想要爬取的日期是?（注意！一次爬取只能设定一次日期，且需按照如20220401这样的格式）";
			cin >> date;
			system("md ETRC");
			num = 1;
		}
		if (date.size() == 8 && AllisNum(date))
		{
			cout << "您想要爬取的车次是?（请输入始发站车次，输入E退出）";
			cin >> train_number;
			if (train_number == "E")
			{
				csv.close();
				cout << endl << "qETRC.csv保存在同目录，可用Excel与记事本打开，导入到qETRC前请使用记事本将文件另存为UTF8编码。" << endl;
				cout << endl << "ETRC车次文件保存在名为ETRC的目录，可用导入到ETRC。" << endl;
				system("pause");
				exit(0);
			}
			search_result = get_search(train_number, date, header, options);
			if (search_result.size() == 39 || search_result.size() == 43)
			{
				cout << "输入有误，请重新输入" << endl;
			}
			else
			{
				cJSON* root = cJSON_Parse(search_result.c_str());
				if (root == NULL)return 0;
				cJSON* item = NULL;
				char* v_str = NULL;
				double v_double = 0.0;
				int v_int = 0;
				bool v_bool = false;
				item = cJSON_GetObjectItem(root, "data");
				if (item != NULL)
				{
					cJSON* obj = cJSON_GetArrayItem(item, 0);
					cJSON* val = NULL;
					if (obj != NULL && obj->type == cJSON_Object)
					{	
						val = cJSON_GetObjectItem(obj, "train_no");
						if (val != NULL && val->type == cJSON_String)
						{
							search["train_no"]  = val->valuestring;
						}
						val = cJSON_GetObjectItem(obj, "station_train_code");
						if (val != NULL && val->type == cJSON_String)
						{
							search["station_train_code"] = val->valuestring;
						}
					}
				}
				if (search["station_train_code"] == train_number)
				{
					cout << endl << "查询到" << search["station_train_code"] << "的代码为"  << search["train_no"] << "。" << endl;
					string yy = date.substr(0, 4);
					string mm = date.substr(4, 2);
					string dd = date.substr(6, 2);
					if (options["proxy"] != "None")
					{
						Response resp = Get("https://kyfw.12306.cn/otn/queryTrainInfo/query?leftTicketDTO.train_no=" + search["train_no"] + "&leftTicketDTO.train_date=" + yy + "-" + mm + "-" + dd + "&rand_code=", header, "", options);
						train_info = resp.GetText();
					}
					else
					{
						Response resp = Get("https://kyfw.12306.cn/otn/queryTrainInfo/query?leftTicketDTO.train_no=" + search["train_no"] + "&leftTicketDTO.train_date=" + yy + "-" + mm + "-" + dd + "&rand_code=", header);
						train_info = resp.GetText();
					}
					cJSON* root = cJSON_Parse(train_info.substr(84, train_info.size()-37).c_str());
					if (root == NULL)
					{
						return 0;
					}
					item = NULL;
					v_str = NULL;
					v_double = 0.0;
					v_int = 0;
					v_bool = false;
					item = cJSON_GetObjectItem(root, "data");
					vector<string> start_end = { "","" };
					vector<string> e_train_number = { "","" };
					string e_list;
					if (item != NULL)
					{
						int size = cJSON_GetArraySize(item);
						for (int i2 = 0; i2 < size; i2++)
						{
							if (item != NULL)
							{
								cJSON* obj = cJSON_GetArrayItem(item, i2);
								cJSON* val = NULL;
								if (obj != NULL && obj->type == cJSON_Object)
								{
									val = cJSON_GetObjectItem(obj, "station_train_code");
									if (val != NULL && val->type == cJSON_String)
									{
										train["station_train_code"] = val->valuestring;
									}val = cJSON_GetObjectItem(obj, "station_name");
									if (val != NULL && val->type == cJSON_String)
									{
										train["station_name"] = val->valuestring;
									}
									val = cJSON_GetObjectItem(obj, "arrive_time");
									if (val != NULL && val->type == cJSON_String)
									{
										train["arrive_time"] = val->valuestring;
									}
									val = cJSON_GetObjectItem(obj, "start_time");
									if (val != NULL && val->type == cJSON_String)
									{
										train["start_time"] = val->valuestring;
									}
									if (train["arrive_time"] == "----")
									{
										train["arrive_time"] = train["start_time"];
									}
									if (train["start_time"] == "----")
									{
										train["start_time"] = train["arrive_time"];
									}
									string banke = "true";
									if (rules_list[0] != "none")
									{
										for (int i1 = 0; i1 < rules_list.size() ; i1++)
										{
											rule = split(rules_list[i1], " ");
											if (train["station_name"] == rule[0])
											{
												if (rule[2] != "0" && rule[3] != "0")
												{
													cout << "请检查规则文件设置是否正确。" << endl;
													system("pause");
													exit(0);
												}
												cout << "将" << train["station_name"] << "更改为" << rule[1] << "，将该站的";
												train["station_name"] = rule[1];
												int m,h1,m1;
												string min, hour;
				
												if (rule[2] != "0")
												{
													cout << "出发时间由" << train["start_time"] << "改为";
													m = atoi(train["start_time"].substr(0, 2).c_str()) * 60 + atoi(train["start_time"].substr(3, 2).c_str()) + atoi(rule[2].c_str());
													if (m > 1440)
													{
														m = m - 1440;
													}
													if (m < 0)
													{
														m = m + 1440;
													}
													h1 = m / 60;
													m1 = m - h1 * 60;
													hour = to_string(h1);
													min = to_string(m1);
													if (hour.size() == 1)
													{
														hour = "0" + hour;
													}
													if (min.size() == 1)
													{
														min = "0" + min;
													}
													train["start_time"] = hour + ":" + min;
													banke = "false";
													cout << train["start_time"] << "并设为通过状态。" << endl;
													train["arrive_time"] = train["start_time"];
												}
												if (rule[3] != "0")
												{
													cout << "到达时间由" << train["arrive_time"] << "改为";
													m = atoi(train["arrive_time"].substr(0, 2).c_str()) * 60 + atoi(train["arrive_time"].substr(3, 2).c_str()) + atoi(rule[3].c_str());
													if (m > 1440)
													{
														m = m - 1440;
													}
													if (m < 0)
													{
														m = m + 1440;
													}
													h1 = m / 60;
													m1 = m - h1 * 60;
													hour = to_string(h1);
													min = to_string(m1);
													if (hour.size() == 1)
													{
														hour = "0" + hour;
													}
													if (min.size() == 1)
													{
														min = "0" + min;
													}
													train["arrive_time"] = hour + ":" + min;
													banke = "false";
													cout << train["arrive_time"] << "并设为通过状态。" << endl;
													train["start_time"] = train["arrive_time"];
												}
											}
										}
									}
									if (i2 == 0)
									{
										start_end.front() = train["station_name"];
									}
									if (i2 == size - 1)
									{
										start_end.back() = train["station_name"];
									}
									cout << "获取到" << train["station_train_code"] << "在" << train["station_name"] << "的到达时间为" << train["arrive_time"] << "，出发时间为" << train["start_time"] << "。" << endl;
									if (csv)
									{
										csv << Utf8ToGbk(train["station_train_code"]) << Utf8ToGbk(",") << Utf8ToGbk(train["station_name"]) << Utf8ToGbk(",") << Utf8ToGbk(train["arrive_time"]) << Utf8ToGbk(",") << Utf8ToGbk(train["start_time"]) << Utf8ToGbk("\n");
									}
									if (count(e_train_number.begin(), e_train_number.end(), train["station_train_code"]) == 0)
									{
										string last_char = train["station_train_code"].substr(train["station_train_code"].size() - 1, 1);
										if (last_char == "0" || last_char == "2" || last_char == "4" || last_char == "6" || last_char == "8" )
										{
											e_train_number.back() = train["station_train_code"];
										}
										else
										{
											e_train_number.front() = train["station_train_code"];
										}
									}
									e_list += train["station_name"] + "," + train["arrive_time"] + "," + train["start_time"] + "," + banke + ",NA,0,\n";
								}
							}
						}
						cout << endl;
					}
					ofstream trf("ETRC\\" + train_number + ".trf");
					if (trf)
					{
						trf.imbue(std::locale(trf.getloc(), new std::codecvt_utf8<wchar_t, 0x10ffff, std::little_endian>));
						trf << "trf2," << train_number << "," << e_train_number.front() << "," << e_train_number.back() << ",NA\n" << start_end.front() << "\n" << start_end.back() << "\n" << e_list;
					}
					trf.close();
				}
				else
				{
					cout << "输入有误，请重新输入" << endl;
				}
			}
		}
		else
		{
		cout << "输入有误，请重新输入" << endl;
		num = 0;
		}
	}
}