#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <string>
#include <format>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cstring>
#include <map>

using namespace std;

template<typename T>
string format_bin(T num) {
	string ret;
	for (int i = sizeof(T)-1; i > -1; i--) {
		for (int j = 0; j < 8; j++) {
			ret += ((char*)&num)[i] & (1<<(7-j)) ? '1' : '0';
		}
	}
	return ret;
}

enum class termios_flags : int {
    iflag,
    oflag,
    cflag,
    lflag
};

template<typename MAP_T, typename T>
string map_print(MAP_T map, T flags) {
	string ret;
	int max = sizeof(T) * 8;
	int cur = 0;

	while (true) {
		int bufsize = max+11;
		char buf[bufsize];
		memset(buf, 0, bufsize);
		memset(buf, ' ', max);
		bool flag = false;
		int cmax=max;
		auto itmax = map.end();
		for (int i=cur; i < max; i++) {
			int ck = 1 << i;
			auto it = map.find(ck);
			if (it != map.end() && flags & ck) {
				buf[max-i-1] = '|';
				flag = true;
				if (i < cmax) {
					cmax=i;
					itmax = it;
				}
			}
		}

		if (!flag)
			break;
		auto str = itmax->second;
		memcpy(buf+max-cmax, str.data(), str.size());
		cur=cmax+1;
		ret += buf;
		ret += '\n';
	}
	return ret;
}

template<termios_flags Flag>
struct TermFormat {
	template<typename T>
	static string format_flags(T flags) {
		return "undefined";
	}
};

template<>
struct TermFormat<termios_flags::iflag> {
	template<typename T>
	static string format_flags(T flags) {
		string ret;
		ret += std::format("\nc_iflags\n{}\n", format_bin(flags));
		map<T, string_view> iflags = {
			{IGNBRK, "IGNBRK"},
			{BRKINT, "BRKINT"},
			{IGNPAR, "IGNPAR"},
			{PARMRK, "PARMRK"},
			{INPCK, "INPCK"},
			{ISTRIP, "ISTRIP"},
			{INLCR, "INLCR"},
			{IGNCR, "IGNCR"},
			{ICRNL, "ICRNL"},
			{IUCLC, "IUCLC"},
			{IXON, "IXON"},
			{IXANY, "IXANY"},
			{IXOFF, "IXOFF"},
			{IMAXBEL, "IMAXBEL"},
			{IUTF8, "IUTF8"}
		};
		ret += map_print(iflags, flags);
		return ret;
	}
};

template<>
struct TermFormat<termios_flags::oflag> {
	template<typename T>
	static string format_flags(T flags) {
		string ret;
		ret += std::format("\nc_oflags\n{}\n", format_bin(flags));
		map<T, string_view> oflags = {
			{OPOST, "OPOST"},
			{OLCUC, "OLCUC"},
			{ONLCR, "ONLCR"},
			{OCRNL, "OCRNL"},
			{ONOCR, "ONOCR"},
			{ONLRET, "ONLRET"},
			{OFILL, "OFILL"},
			{OFDEL, "OFDEL"},
			{NLDLY, "NLDLY"},
			{CRDLY, "CRDLY"},
			{TABDLY, "TABDLY"},
			{BSDLY, "BSDLY"},
			{VTDLY, "VTDLY"},
			{FFDLY, "FFDLY"}
		};
		ret += map_print(oflags, flags);
		return ret;
	}
};

template<>
struct TermFormat<termios_flags::cflag> {
	template<typename T>
	static string format_flags(T flags) {
		string ret;
		ret += std::format("\nc_cflags\n{}\n", format_bin(flags));
		map<T, string_view> cflags = {
			{CBAUD, "CBAUD"},
			{CBAUDEX, "CBAUDEX"},
			{CSIZE, "CSIZE"},
			{CSTOPB, "CSTOPB"},
			{CREAD, "CREAD"},
			{PARENB, "PARENB"},
			{PARODD, "PARODD"},
			{HUPCL, "HUPCL"},
			{CLOCAL, "CLOCAL"},
			{CIBAUD, "CIBAUD"},
			{CMSPAR, "CMSPAR"},
			{CRTSCTS, "CRTSCTS"}
		};
		ret += map_print(cflags, flags);
		return ret;
	}
};

template<>
struct TermFormat<termios_flags::lflag> {
	template<typename T>
	static string format_flags(T flags) {
		string ret;
		ret += std::format("\nc_lflags\n{}\n", format_bin(flags));
		map<T, string_view> lflags = {
			{ISIG, "ISIG"},
			{ICANON, "ICANON"},
			{XCASE, "XCASE"},
			{ECHO, "ECHO"},
			{ECHOE, "ECHOE"},
			{ECHOK, "ECHOK"},
			{ECHONL, "ECHONL"},
			{ECHOCTL, "ECHOCTL"},
			{ECHOPRT, "ECHOPRT"},
			{ECHOKE, "ECHOKE"},
			{FLUSHO, "FLUSHO"},
			{NOFLSH, "NOFLSH"},
			{TOSTOP, "TOSTOP"},
			{PENDIN, "PENDIN"},
			{IEXTEN, "IEXTEN"}
		};
		ret += map_print(lflags, flags);
		return ret;
	}
};

int main(int argc, char** argv) {
	termios term_s;
	termios *term = &term_s;
	tcgetattr(STDIN_FILENO, term);

    cout << TermFormat<termios_flags::iflag>::format_flags(term->c_iflag);
	cout << TermFormat<termios_flags::oflag>::format_flags(term->c_oflag);
	cout << TermFormat<termios_flags::cflag>::format_flags(term->c_cflag);
	cout << TermFormat<termios_flags::lflag>::format_flags(term->c_lflag);
	
	return 0;
}
