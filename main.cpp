#include <iostream>
#include <algorithm>
//#include <cctype>
//#include <cstddef>
//#include <cstring>
//#include <stdio.h>
//#include <cstdlib>
#include <windows.h>
//#include <stdlib.h>
//#include <cstring>
//#include <ctime>
//#include <time.h>
//#include <math.h>
//#include <conio.h>
//#include <dir.h>
#include <string>
#include <sstream>


int off_code(int number){
    if (number < -2048)
        number = -2048;
    else if (number > 2047)
        number = 2047;

    number += 2048;

//    number = number ^ 0x800;

    return number;
}


std::string make_string(int number){
    if (number < 0)
        number = 0;
    else if (number > 4095)
        number = 4095;

    std::stringstream ss;
    ss << std::hex << number;

    std::string num_hex = ss.str();
	std::cout<<"num_hex: "<<num_hex<<std::endl;

    std::string zero = "0";

	std::cout<<"ss: "<<ss.str()<<std::endl;
    if (num_hex.length() == 1){
    	num_hex = zero.append(zero).append(ss.str());
    	std::cout<<"length: 1 num_hex: "<<num_hex<<std::endl;
    	
	}
        
    else if (num_hex.length() == 2) {
    	num_hex = zero.append(ss.str());
    	std::cout<<"length: 2 num_hex: "<<num_hex<<std::endl;
	}
    return num_hex;
}


std::string configure_for_wavelength(int int_wave){
    int ofsetted = off_code(int_wave);
    std::cout<<"Offsetted: "<<ofsetted<<std::endl;
    std::string string_code = make_string(ofsetted);
    std::cout<<"string_code: "<<string_code<<std::endl;
    std::string i_4 = "I4";
    std::string to_be_sent = i_4.append(string_code).append("P1P0");
    std::transform(to_be_sent.begin(), to_be_sent.end(), to_be_sent.begin(), ::toupper);
    std::cout<<"to_be_sent: "<<to_be_sent<<std::endl;
    return to_be_sent;
}


int main()
{
	char buffer[4];
	DWORD read = 0;
    std::string input;
    int step;
	std::ostringstream readstream;
	DWORD write=0; int i;
    int sucess;
    char w1[]={'1','W','S','\r'};
    char w2[]={'2','W','S','\r'};
    HANDLE h = CreateFile("\\\\.\\COM1",GENERIC_READ|GENERIC_WRITE, 0,NULL,OPEN_EXISTING,0,NULL);
    COMMTIMEOUTS cto = { 1, 100, 1000, 0, 0 };
    DCB dcb;
    SetCommTimeouts(h,&cto);
    memset(&dcb,0,sizeof(dcb));
    dcb.DCBlength = sizeof(dcb);
    dcb.BaudRate = 9600;
    dcb.fBinary = 1;
    dcb.Parity = ODDPARITY;
    dcb.StopBits = ONESTOPBIT;
    dcb.ByteSize = 7;
    SetCommState(h,&dcb);
    char ssa[]={'!','Q','T','\r'};
    sucess=WriteFile(h,ssa,sizeof(ssa),&write,NULL);
    char ssb[]={'P','O','\r'};
    sucess=WriteFile(h,ssb,sizeof(ssb),&write,NULL);
    char ssc[]={'I','7','0','0','0','P','1','P','0','\r'};
    sucess=WriteFile(h,ssc,sizeof(ssc),&write,NULL);
    char ssd[]={'I','0','\r'};
    sucess=WriteFile(h,ssd,sizeof(ssd),&write,NULL);
    char sse[]={'O','3','\r'};
    sucess=WriteFile(h,sse,sizeof(sse),&write,NULL);
    while (1) {
        std::cout<<"Enter e for exit or r for read or b for balance mode or o for operate mode or enter value between -2048 and 2047"<<std::endl;
        getline(std::cin, input);
        if (input[0] == 'e' || input[0] == 'E') {
            break;
        }
        else if (input[0] == 'r' || input[0] == 'R') {
        	std::string to_be_sent;
			to_be_sent = "?";
        	to_be_sent.append("\r");
        	sucess=WriteFile(h,to_be_sent.c_str(),sizeof(to_be_sent.c_str()),&write,NULL);
            read = 0;
			ReadFile(h, buffer, sizeof(buffer), &read, NULL);
            readstream<<read;
            std::string out = readstream.str();
//            std::string word = out.substr(1, std::string::npos);
//            std::string x_0 = "0x";
//            word = x_0.append(word);
//            int int_word = atoi(word.c_str());
//            int_word = int_word ^ 0x800;
            std::cout<<"The word read is :"<<out<<std::endl;
            continue;
        }
        else if (input[0] == 'b' || input[0] == 'B') {
        	std::string to_be_sent;
			to_be_sent = "O1";
        	to_be_sent.append("\r");
        	sucess=WriteFile(h,to_be_sent.c_str(),sizeof(to_be_sent.c_str()),&write,NULL);
            continue;
        }
        else if (input[0] == 'o' || input[0] == 'O') {
        	std::string to_be_sent;
			to_be_sent = "O0";
        	to_be_sent.append("\r");
        	sucess=WriteFile(h,to_be_sent.c_str(),sizeof(to_be_sent.c_str()),&write,NULL);
            continue;
        }
        step = atoi(input.c_str());
        std::string to_be_sent;
        if (step < -2048 || step > 2047) {
        	std::cout<<"Valid values are between -2048 and 2047"<<std::endl;
        	continue;
		}
		if (step >= 0) {
			step = step - 2048;
		}
		else {
			step = step + 2048;
		}
		std::cout<<"The value of Step to be applied is: "<<step<<std::endl;
		to_be_sent = configure_for_wavelength(step);
//        to_be_sent.append("\r");
        std::cout<<"The final word to be sent is: "<<to_be_sent<<std::endl;
        char sending[] = {
        	to_be_sent[0],
        	to_be_sent[1],
        	to_be_sent[2],
        	to_be_sent[3],
        	to_be_sent[4],
        	to_be_sent[5],
        	to_be_sent[6],
        	to_be_sent[7],
        	to_be_sent[8],
        	'\r'
		};
        //char sst[]={'I','7','0','0','0','P','1','P','0','\r'};
    	//sucess=WriteFile(h,sst,sizeof(sst),&write,NULL);
//       char sse[]={'I','4','8','0','A','P','1','P','0','\r'};
//       sucess=WriteFile(h,sse,sizeof(sse),&write,NULL);
        sucess=WriteFile(h,sending,sizeof(sending),&write,NULL);
		if (write != sizeof(sending) ){
			std::cout<<"Not all data written to port"<<std::endl;
		}
		to_be_sent = "I0";
        to_be_sent.append("\r");
        sucess=WriteFile(h,to_be_sent.c_str(),sizeof(to_be_sent.c_str()),&write,NULL);
    }
    std::string to_be_sent;
	to_be_sent = "O2";
    to_be_sent.append("\r");
    sucess=WriteFile(h,to_be_sent.c_str(),sizeof(to_be_sent.c_str()),&write,NULL);
    return 0;
}

