#include<windows.h>
#include<iostream>
#include<vector>
#include<codecvt>
#include<cpr/cpr.h>
#pragma execution_character_set("utf-8")
#define true 1
#define false 0
#include"cJSON.h"
using namespace std;
string Lpcwstr2String(LPCWSTR lps) {
	int len = WideCharToMultiByte(CP_ACP, 0, lps, -1, NULL, 0, NULL, NULL);
	if (len <= 0) {
		return "";
	}
	else {
		char* dest = new char[len];
		WideCharToMultiByte(CP_ACP, 0, lps, -1, dest, len, NULL, NULL);
		dest[len - 1] = 0;
		string str(dest);
		delete[] dest;
		return str;
	}
}
string get_path() {
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
	if (GetOpenFileName(&ofn)) {
		path_image = Lpcwstr2String(ofn.lpstrFile);
		return path_image;
	}
	else {
		return "";
	}
}
bool AllisNum(string str) {
	for (int i = 0; i < str.size(); i++) {
		int tmp = (int)str[i];
		if (tmp >= 48 && tmp <= 57) {
			continue;
		}
		else {
			return false;
		}
	}
	return true;
}
vector<string> split(string str, string pattern) {
	string::size_type pos;
	vector<string> result;
	str += pattern;
	//扩展字符串以方便操作
	int size = str.size();
	for (int i = 0; i < size; i++) {
		pos = str.find(pattern, i);
		if (pos < size) {
			string s = str.substr(i, pos - i);
			result.push_back(s);
			i = pos + pattern.size() - 1;
		}
	}
	return result;
}

ofstream csv("train.csv");
int main() {
	system("chcp 65001");
	system("md ETRC");
	string date;
	string train_info;
	string train_number;
	string search_result;
	string timetable;
	string pyetdb = "{\"line\": {\"name\": \"\", \"rulers\": [], \"routes\": [], \"stations\": [], \"forbid\": {\"different\": true, \"nodes\": [], \"upShow\": false, \"downShow\": false}, \"forbid2\": {\"different\": true, \"nodes\": [], \"upShow\": false, \"downShow\": false}, \"notes\": {\"author\":\"\", \"version\": \"\", \"note\": \"\"}, \"tracks\": []}, \"trains\": [";
	string path;
	string proxy;
	string hcxt = "";
	char use_rules;
	char use_porxies;
	char txt[100];
	ifstream infile;
	vector<string> rule;
	vector<string> rules_list;
	vector<string> trains_list;
	map<string, string> train;
	cJSON* item = NULL;
	char* v_str = NULL;
	double v_double = 0.0;
	int v_int = 0;
	int skbbbh;
	bool v_bool = false;
	cout << "欢迎使用cParsing_12306，本项目使用并遵循GPLv3协议，程序作者：denglihong2007。";
	cout << "请问您是否需要导入规则文件（UTF-8编码，格式为A B 1 0，意为将A站改为B站且把到达时间推迟一分钟设为通过状态）？（是的话输入Y，否则输入其它键）";
	cin >> use_rules;
	if ('Y' == use_rules || 'y' == use_rules) {
		infile.open(get_path());
		if (!infile.is_open()) {
			cout << "打开失败。" << endl;
			system("pause");
			exit(0);
		}
		while (!infile.eof()) {
			infile.getline(txt, 100);
			rules_list.push_back(txt);
		}
		infile.close();
	}
	else {
		rules_list.push_back("none");
	}
	cout << "请问您是否需要设置网络代理？（是的话输入Y，否则输入其它键）";
	cin >> use_porxies;
	if ('Y' == use_porxies || 'y' == use_porxies) {
		cout << "请输入代理http地址。http://";
		cin >> proxy;
		proxy = "http://" + proxy;
	}
	else {
		proxy = "None";
	}
	int num = 0;
	while (num < 2) {
		if (num == 0) {
			cout << "您想要爬取的日期是?（注意！一次爬取只能设定一次日期，且需按照如20220401这样的格式）";
			cin >> date;
			num = 1;
		}
		if (date.size() == 8 && AllisNum(date)) {
			cout << "您想要爬取的车次是?（输入E退出）";
			cin >> train_number;
			timetable = "\"timetable\": [";
			if (train_number == "E") {
				csv.close();
				pyetdb = pyetdb.substr(0, pyetdb.length() - 2) + "], \"circuits\": [], \"config\": {\"ordinate\": null, \"not_show_types\": [], \"seconds_per_pix\": 15.0, \"seconds_per_pix_y\": 8.0, \"pixes_per_km\": 4.0, \"grid_color\": \"#AAAA7F\", \"text_color\": \"#0000FF\", \"default_keche_width\": 1.5, \"default_huoche_width\": 0.75, \"default_db_file\": \"linesNew.pyetlib\", \"start_hour\": 0, \"end_hour\": 24, \"minutes_per_vertical_line\": 10.0, \"bold_line_level\": 2, \"show_line_in_station\": true, \"start_label_height\": 30, \"end_label_height\": 15, \"table_row_height\": 30, \"link_line_height\": 10, \"show_time_mark\": 1, \"max_passed_stations\": 3, \"avoid_cover\": true, \"base_label_height\": 15, \"step_label_height\": 20, \"end_label_checi\": false, \"default_colors\": {\"快速\": \"#FF0000\", \"特快\": \"#0000FF\", \"直达特快\": \"#FF00FF\", \"动车组\": \"#804000\", \"动车\": \"#804000\", \"高速\": \"#FF00BE\", \"城际\": \"#FF33CC\", \"default\": \"#008000\"}, \"margins\": {\"left_white\": 15, \"right_white\": 10, \"left\": 275, \"up\": 90, \"down\": 90, \"right\": 150, \"label_width\": 80, \"mile_label_width\": 40, \"ruler_label_width\": 80}, \"type_regex\": [[\"高速\", \"G\\\\d+\", true], [\"动车组\", \"D\\\\d+\", true], [\"城际\", \"C\\\\d+\", true], [\"直达特快\", \"Z\\\\d+\", true], [\"特快\", \"T\\\\d+\", true], [\"快速\", \"K\\\\d+\", true], [\"普快\", \"[1-5]\\\\d{3}$\", true], [\"普快\", \"[1-5]\\\\d{3}\\\\D\", true], [\"普客\", \"6\\\\d{3}$\", true], [\"普客\", \"6\\\\d{3}\\\\D\", true], [\"普客\", \"7[1-5]\\\\d{2}$\", true], [\"普客\", \"7[1-5]\\\\d{2}\\\\D\", true], [\"通勤\", \"7\\\\d{3}$\", true], [\"通勤\", \"7\\\\d{3}\\\\D\", true], [\"通勤\", \"8\\\\d{3}$\", true], [\"通勤\", \"8\\\\d{3}\\\\D\", true], [\"旅游\", \"Y\\\\d+\", true], [\"路用\", \"57\\\\d+\", true], [\"特快行包\", \"X1\\\\d{2}\", true], [\"动检\", \"DJ\\\\d+\", true], [\"客车底\", \"0[GDCZTKY]\\\\d+\", true], [\"临客\", \"L\\\\d+\", true], [\"客车底\", \"0\\\\d{4}\", true], [\"行包\", \"X\\\\d{3}\\\\D\", false], [\"行包\", \"X\\\\d{3}$\", false], [\"班列\", \"X\\\\d{4}\", false], [\"直达\", \"1\\\\d{4}\", false], [\"直货\", \"2\\\\d{4}\", false], [\"区段\", \"3\\\\d{4}\", false], [\"摘挂\", \"4[0-4]\\\\d{3}\", false], [\"小运转\", \"4[5-9]\\\\d{3}\", false], [\"单机\", \"5[0-2]\\\\d{3}\", false], [\"补机\", \"5[3-4]\\\\d{3}\", false], [\"试运转\", \"55\\\\d{3}\", false]]}, \"version\": \"\", \"markdown\": \"\"}";
				ofstream pyetdb_f("train.pyetdb");
				if (pyetdb_f) {
					pyetdb_f.imbue(std::locale(pyetdb_f.getloc(), new std::codecvt_utf8<wchar_t, 0x10ffff, std::little_endian>));
					pyetdb_f << pyetdb;
				}
				pyetdb_f.close();
				cout << endl << "train.csv保存在同目录，可用qETRC与记事本打开，导入到EXCEL前请使用记事本将文件另存为ANSI编码。" << endl;
				cout << "Pyetdb车次文件保存在同目录，可用导入到qETRC与pyETRC。已知问题导入车次后部分车站不为“营业”，请手动修改。" << endl;
				cout << "ETRC车次文件保存在名为ETRC的目录，可用导入到ETRC。" << endl;
				system("pause");
				exit(0);
			}
			else {
				if (count(trains_list.begin(), trains_list.end(), train_number) == 0) {
					if ((AllisNum(train_number) && train_number.length() == 5) || train_number.substr(0, 1) == "X" || train_number.find("/") != string::npos) {
						vector<string> bllx = { "01", "02", "03", "05" };
						for (int i = 0; i < bllx.size(); i++) {
							if (proxy != "None") {
								cpr::Response r = cpr::Post(cpr::Url{ "https://ec.95306.cn/api/bl/queryProduct/queryProductQuery" },
									cpr::Timeout{ 7000 },
									cpr::Header{ {"Content-Type", "application/json"} },
									cpr::Proxies{ {"https", proxy} },
									cpr::Body{ R"({"pageNum":1,"pageSize":10,"bllx":")" + bllx[i] + R"(","cprq":")" + date.substr(0,4) + "-" + date.substr(4,2) + "-" + date.substr(6,2) + R"(","zcztmism":"","zczItem" : "","zcz" : "","fashi" : "","gbcc" : ")" + train_number + R"(","xcztmism" : "","xczItem" : "","xcz" : "","daoshi" : "","fjhz" : "","fj" : "","fjItem" : "","dj" : "","djdm" : "","djItem" : "","djhz" : ""})" });
								train_info = r.text;
							}
							else {
								cpr::Response r = cpr::Post(cpr::Url{ "https://ec.95306.cn/api/bl/queryProduct/queryProductQuery" },
									cpr::Timeout{ 7000 },
									cpr::Header{ {"Content-Type", "application/json"} },
									cpr::Body{ R"({"pageNum":1,"pageSize":10,"bllx":")" + bllx[i] + R"(","cprq":")" + date.substr(0,4) + "-" + date.substr(4,2) + "-" + date.substr(6,2) + R"(","zcztmism":"","zczItem" : "","zcz" : "","fashi" : "","gbcc" : ")" + train_number + R"(","xcztmism" : "","xczItem" : "","xcz" : "","daoshi" : "","fjhz" : "","fj" : "","fjItem" : "","dj" : "","djdm" : "","djItem" : "","djhz" : ""})" });
								train_info = r.text;
							}
							if (train_info.find(R"("list":[])") == string::npos) {
								break;
							}
						}
						if (train_info.find(R"(list":[])") == string::npos) {
							skbbbh = cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_Parse(train_info.c_str()), "data"), "list"), 0), "skbbbh")->valueint;
							if (cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_Parse(train_info.c_str()), "data"), "list"), 0), "gbcc")->valuestring != train_number) {
								hcxt = "F";
							}
							else {
								if (proxy != "None") {
									cpr::Response r = cpr::Post(cpr::Url{ "https://ec.95306.cn/api/bl/queryProduct/querySkb" },
										cpr::Timeout{ 7000 },
										cpr::Header{ {"Content-Type", "application/json"} },
										cpr::Proxies{ {"https", proxy} },
										cpr::Body{ R"({gbcc: ")" + train_number + R"(", skbbbh: )" + to_string(skbbbh) + "}" });
									train_info = r.text;
								}
								else {
									cpr::Response r = cpr::Post(cpr::Url{ "https://ec.95306.cn/api/bl/queryProduct/querySkb" },
										cpr::Timeout{ 7000 },
										cpr::Header{ {"Content-Type", "application/json"} },
										cpr::Body{ R"({gbcc: ")" + train_number + R"(", skbbbh: )" + to_string(skbbbh) + "}" });
									train_info = r.text;
								}

							}
						}
						else {
							hcxt = "F";
						}
					}
					else {
						if (proxy != "None") {
							cpr::Response r = cpr::Get(cpr::Url{ "https://wifi.12306.cn/wifiapps/ticket/api/stoptime/queryByTrainCode?trainCode=" + train_number + "&trainDate=" + date + "&getBigScreen=true" },
								cpr::Header{ {"User-Agent", "Mozilla/5.0 (Linux; Android 12; Redmi Note 8 Build/SP2A.220405.004; wv) AppleWebKit/537.36 (KHTML, like Gecko) Version/4.0 Chrome/86.0.4240.99 XWEB/3235 MMWEBSDK/20220402 Mobile Safari/537.36 MMWEBID/1660 MicroMessenger/8.0.22.2140(0x28001637) WeChat/arm64 Weixin NetType/WIFI Language/zh_CN ABI/arm64 MiniProgramEnv/android"} },
								cpr::Timeout{ 7000 },
								cpr::Proxies{ {"https", proxy} });
							train_info = r.text;
						}
						else {
							cpr::Response r = cpr::Get(cpr::Url{ "https://wifi.12306.cn/wifiapps/ticket/api/stoptime/queryByTrainCode?trainCode=" + train_number + "&trainDate=" + date + "&getBigScreen=true" },
								cpr::Header{ {"User-Agent", "Mozilla/5.0 (Linux; Android 12; Redmi Note 8 Build/SP2A.220405.004; wv) AppleWebKit/537.36 (KHTML, like Gecko) Version/4.0 Chrome/86.0.4240.99 XWEB/3235 MMWEBSDK/20220402 Mobile Safari/537.36 MMWEBID/1660 MicroMessenger/8.0.22.2140(0x28001637) WeChat/arm64 Weixin NetType/WIFI Language/zh_CN ABI/arm64 MiniProgramEnv/android"} },
								cpr::Timeout{ 7000 });
							train_info = r.text;
						}
					}
					if (train_info.size() == 111 || train_info.size() == 107 || hcxt == "F") {
						cout << "当日未查找到此车次。" << endl;
					}
					else {
						cJSON* root = cJSON_Parse(train_info.c_str());
						item = cJSON_GetObjectItem(root, "data");
						vector<string> start_end = {
							"",""
						}
						;
						vector<string> e_train_number = {
							"",""
						}
						;
						string e_list;
						if (item != NULL) {
							int size = cJSON_GetArraySize(item);
							for (int i2 = 0; i2 < size; i2++) {
								if (item != NULL) {
									cJSON* obj = cJSON_GetArrayItem(item, i2);
									cJSON* val = NULL;
									if (obj != NULL && obj->type == cJSON_Object) {
										val = cJSON_GetObjectItem(obj, "ddsj");
										if (val != NULL && val->type == cJSON_String) {
											train["arriveTime"] = val->valuestring;
										}
										val = cJSON_GetObjectItem(obj, "cfsj");
										if (val != NULL && val->type == cJSON_String) {
											train["startTime"] = val->valuestring;
										}
										val = cJSON_GetObjectItem(obj, "czhzzm");
										if (val != NULL && val->type == cJSON_String) {
											train["stationName"] = val->valuestring;
											train["stationTrainCode"] = train_number;
											if (train["startTime"] == "---") {
												train["startTime"] = train["arriveTime"];
											}
											if (train["arriveTime"] == "---") {
												train["arriveTime"] = train["startTime"];
											}
										}
										val = cJSON_GetObjectItem(obj, "stationTrainCode");
										if (val != NULL && val->type == cJSON_String) {
											train["stationTrainCode"] = val->valuestring;
										}
										val = cJSON_GetObjectItem(obj, "stationName");
										if (val != NULL && val->type == cJSON_String) {
											train["stationName"] = val->valuestring;
										}
										val = cJSON_GetObjectItem(obj, "arriveTime");
										if (val != NULL && val->type == cJSON_String) {
											train["arriveTime"] = val->valuestring;
											train["arriveTime"] = train["arriveTime"].substr(0, 2) + ":" + train["arriveTime"].substr(2, 4);
										}
										val = cJSON_GetObjectItem(obj, "startTime");
										if (val != NULL && val->type == cJSON_String) {
											train["startTime"] = val->valuestring;
											train["startTime"] = train["startTime"].substr(0, 2) + ":" + train["startTime"].substr(2, 4);
										}
										string banke = "true";
										if (rules_list[0] != "none") {
											for (int i1 = 0; i1 < rules_list.size(); i1++) {
												rule = split(rules_list[i1], " ");
												if (train["stationName"] == rule[0]) {
													if (rule[2] != "0" && rule[3] != "0") {
														cout << "请检查规则文件设置是否正确。" << endl;
														system("pause");
														exit(0);
													}
													cout << "将" << train["stationName"] << "更改为" << rule[1] << "，将该站的";
													train["stationName"] = rule[1];
													int m, h1, m1;
													string min, hour;
													if (rule[2] != "0") {
														cout << "出发时间由" << train["startTime"] << "改为";
														m = atoi(train["startTime"].substr(0, 2).c_str()) * 60 + atoi(train["startTime"].substr(3, 2).c_str()) + atoi(rule[2].c_str());
														if (m > 1440) {
															m = m - 1440;
														}
														if (m < 0) {
															m = m + 1440;
														}
														h1 = m / 60;
														m1 = m - h1 * 60;
														hour = to_string(h1);
														min = to_string(m1);
														if (hour.size() == 1) {
															hour = "0" + hour;
														}
														if (min.size() == 1) {
															min = "0" + min;
														}
														train["startTime"] = hour + ":" + min;
														banke = "false";
														cout << train["startTime"] << "并设为通过状态。" << endl;
														train["arriveTime"] = train["startTime"];
													}
													if (rule[3] != "0") {
														cout << "到达时间由" << train["arriveTime"] << "改为";
														m = atoi(train["arriveTime"].substr(0, 2).c_str()) * 60 + atoi(train["arriveTime"].substr(3, 2).c_str()) + atoi(rule[3].c_str());
														if (m > 1440) {
															m = m - 1440;
														}
														if (m < 0) {
															m = m + 1440;
														}
														h1 = m / 60;
														m1 = m - h1 * 60;
														hour = to_string(h1);
														min = to_string(m1);
														if (hour.size() == 1) {
															hour = "0" + hour;
														}
														if (min.size() == 1) {
															min = "0" + min;
														}
														train["arriveTime"] = hour + ":" + min;
														banke = "false";
														cout << train["arriveTime"] << "并设为通过状态。" << endl;
														train["startTime"] = train["arriveTime"];
													}
												}
											}
										}
										if (i2 == 0) {
											start_end.front() = train["stationName"];
										}
										if (i2 == size - 1) {
											start_end.back() = train["stationName"];
										}
										cout << "获取到" << train["stationTrainCode"] << "在" << train["stationName"] << "的到达时间为" << train["arriveTime"] << "，出发时间为" << train["startTime"] << "。" << endl;
										if (csv) {
											csv.imbue(std::locale(csv.getloc(), new std::codecvt_utf8<wchar_t, 0x10ffff, std::little_endian>));
											csv << train["stationTrainCode"] << "," << train["stationName"] << "," << train["arriveTime"] << "," << train["startTime"] << "\n";
										}
										if (count(e_train_number.begin(), e_train_number.end(), train["stationTrainCode"]) == 0) {
											string last_char = train["stationTrainCode"].substr(train["stationTrainCode"].size() - 1, 1);
											if (last_char == "0" || last_char == "2" || last_char == "4" || last_char == "6" || last_char == "8") {
												e_train_number.back() = train["stationTrainCode"];
											}
											else {
												e_train_number.front() = train["stationTrainCode"];
											}
										}
										e_list += train["stationName"] + "," + train["arriveTime"] + "," + train["startTime"] + "," + banke + ",NA,0,\n";
										if (i2 == size - 1) {
											timetable = timetable + "{\"zhanming\": \"" + train["stationName"] + "\", \"ddsj\": \"" + train["arriveTime"] + ":00\", \"cfsj\": \"" + train["startTime"] + ":00\", \"note\": \"\", \"track\": \"\", \"business\": true}], \"sfz\": \"";
										}
										else {
											timetable = timetable + "{\"zhanming\": \"" + train["stationName"] + "\", \"ddsj\": \"" + train["arriveTime"] + ":00\", \"cfsj\": \"" + train["startTime"] + ":00\", \"note\": \"\", \"track\": \"\", \"business\": true}, ";
										}
									}
								}
							}
						}
						ofstream trf("ETRC\\" + train_number + ".trf");
						if (trf) {
							trf.imbue(std::locale(trf.getloc(), new std::codecvt_utf8<wchar_t, 0x10ffff, std::little_endian>));
							trf << "trf2," << train_number << "," << e_train_number.front() << "," << e_train_number.back() << ",NA\n" << start_end.front() << "\n" << start_end.back() << "\n" << e_list;
						}
						trf.close();
						pyetdb = pyetdb + "{\"checi\": [\"" + train_number + "\", \"" + e_train_number.front() + "\", \"" + e_train_number.back() + "\"] , \"UI\" : {}, \"type\" : \"\", " + timetable + start_end.front() + "\", \"zdz\": \"" + start_end.back() + "\", \"shown\": true, \"localFirst\" : null, \"localLast\" : null, \"autoItem\" : true, \"itemInfo\" : [] , \"passenger\" : 1, \"carriageCircuit\" : null}, ";
						trains_list.push_back(e_train_number.front());
						trains_list.push_back(e_train_number.back());
					}
				}
				else {
					cout << "车次已存在。" << endl;
				}
			}
		}
		else {
			cout << "日期输入有误，请重新输入" << endl;
			num = 0;
		}
	}
}