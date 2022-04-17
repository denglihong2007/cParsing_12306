#include<windows.h>
#include<string.h>
#include<iostream>
#include<string>
#include<stdlib.h>
#include<fstream>
#include<vector>
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

int main()
{
	system("chcp 65001");
	string date;
	string train_info;
	string train_number;
	string search_result;
	string porxies;
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

    cout << "欢迎使用cParsing_12306，本项目使用并遵循GPLv3协议，程序作者：denglihong2007。请注意爬取车次时请输入始发站车次。\n";
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
			num = 1;
		}
		if (date.size() == 8 && AllisNum(date))
		{
			cout << "您想要爬取的车次是?";
			cin >> train_number;
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
				int i;
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
					cout << "查询到" << search["station_train_code"] << "的代码为"  << search["train_no"] << "。" << endl;
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
					cJSON* root = cJSON_Parse(train_info.c_str());
					if (root == NULL)return 0;
					cJSON* item = NULL;
					char* v_str = NULL;
					double v_double = 0.0;
					int v_int = 0;
					bool v_bool = false;
					int i;
					item = cJSON_GetObjectItem(root, "data");
					if (item != NULL)
					{
						int size = cJSON_GetArraySize(item);
						for (int i = 0; i < size; i++)
						{
							if (item != NULL)
							{
								cJSON* obj = cJSON_GetArrayItem(item, i);
								cJSON* val = NULL;
								if (obj != NULL && obj->type == cJSON_Object)
								{
									val = cJSON_GetObjectItem(obj, "station_train_code");
									if (val != NULL && val->type == cJSON_String)
									{
										train["station_train_code"] = val->valuestring;
									}
									val = cJSON_GetObjectItem(obj, "station_name");
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
									if (rules_list[0] != "none")
									{
										for (int i1 = 0; i1 < rules_list.size() + 1; i1++)
										{
											rule = split(rules_list[i1], " ");
											if (train["station_name"] == rule[0])
											{
												train["station_name"] = rule[1];
												if (rule[2] != "0")
												{
													//这里有一段时间运算
													train["start_time"] = train["arrive_time"];
												}
												if (rule[3] != "0")
												{
													//这里有一段时间运算
													train["arrive_time"] = train["start_time"];
												}
											}
										}
									}
									//这里有一段制作csv的程序
								}
							}
						}
					}
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