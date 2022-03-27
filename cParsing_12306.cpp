#include<windows.h>
#include<string.h>
#include<iostream>
#include<string>
#include<stdlib.h>
#include<fstream>
using namespace std;

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

int main()
{
    string date;
    char use_rules;
    cout << "欢迎使用cParsing_12306，本项目使用并遵循GPLv3协议，程序作者：denglihong2007。\n";
    cout << "请问您是否需要导入规则文件？（是的话输入Y，且文件需要为ANSI编码格式）";
    cin >> use_rules;

    if ('Y' == use_rules || 'y' == use_rules)
    {
		ifstream fIn(get_path());
		if (fIn)
		{
			string str;
			while (getline(fIn, str))
			{
				cout << str << endl;
			}
		}
		else
		{
			cout << "打开失败。" << endl;
		}
		fIn.close();
    }

	while (date.size() != 8)
	{
		cout << "您想要爬取的日期是（注意！一次爬取只能设定一次日期，且需按照如20220401这样的格式）：";
		cin >> date;
		if (date.size() != 8)
		{
			cout << "输入有误，请重新输入。\n";
		}
	}
}