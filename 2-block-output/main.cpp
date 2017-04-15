#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

class blockoutputbuf : public streambuf {
private:
	ostream *out;
	vector<char_type> buffer;
	string startb, endb;
protected:
	virtual int overflow(int c) override {
		if (out->good() && c != traits_type::eof()){
			*pptr() = c; //тут нам пригодился 1 "лишний" символ, убранный в конструкторе
			pbump(1); //смещаем указатель позиции буфера на реальный конец буфера
			return sync() == 0 ? c : traits_type::eof();
		}

		return traits_type::eof();
	}
	
	virtual int sync() override {
		if (pptr() == pbase()) //если буфер пуст, то и синхронизировать нечего
			return 0;

		ptrdiff_t sz = pptr() - pbase(); //вычисляем, сколько символов записано в буффер

		//заворачиваем буфер в наш блок
		*out << startb;
		out->write(pbase(), sz);
		*out << endb;
		
		if (out->good()){
			pbump(-sz); //при успехе перемещаем указатель позиции буфера в начало
			return 0;
		}
		
		return -1;
	}
public:
	blockoutputbuf(ostream &_out, size_t _bufsize, string _startb, string _endb)
		: out(&_out), buffer(_bufsize), startb(_startb), endb(_endb)
	{
		char_type *buf = buffer.data();
		setp(buf, buf + (buffer.size() - 1)); // -1 для того, чтобы упростить реализацию overflow()
	}
};

int main(int argc, char **argv){
	const char str1[] = "In 4 bytes contains 32 bits";
	const char str2[] = "Unix time starts from Jan 1, 1970";
	
	blockoutputbuf buf(cout, 10, "<start>", "<end>\n");
	ostream blockoutput(&buf);
	
	cout << "Original: '" << str1 << "'" << endl;
	cout << "Written to blockoutputbuf: '";
	blockoutput << str1;
	blockoutput.flush(); //"сбросить" то, что не отправлено на консоль из str1
	cout << "'" << endl;
	
	cout << "Original: '" << str2 << "'" << endl;
	cout << "Written to blockoutputbuf: '";
	blockoutput << str2;
	blockoutput.flush();
	cout << "'" << endl;
	
	return 0;
}
