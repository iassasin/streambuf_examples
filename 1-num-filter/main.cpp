#include <iostream>
#include <sstream>
#include <string>

using namespace std;

class numfilterbuf : public streambuf {
private:
	istream *in;
	ostream *out;
	
	int cur; //последнее считанное значение, используется в underflow()
protected:

	/* функции записи в поток: */
	
	virtual int overflow(int c) override {
		if (c == traits_type::eof()){
			return traits_type::eof();
		}
		
		char_type ch = static_cast<char_type>(c);
		if (ch == ' ' || (ch >= '0' && ch <= '9')){ // отдаем пробелы и цифры
			out->put(ch);
			//если что-то не записалось, отдаем EOF
			return out->good() ? ch : traits_type::eof();
		}
		
		return ch;
	}
	
	/* функции чтения из потока: */
	
	//реализация по-умолчанию инкрементирует позицию указателя в буфере и вызывает segmentation fault
	virtual int uflow() override {
		int c = underflow();
		cur = traits_type::eof(); //говорим underflow() считать следующий символ при следующем вызове
		return c;
	}
	
	virtual int underflow() override {
		if (cur != traits_type::eof()){
			return cur;
		}
		
		// пока можем читать, читаем
		while (in->good()){
			cur = in->get();
			if (cur == traits_type::eof()){
				return traits_type::eof();
			}
			
			char_type ch = static_cast<char_type>(cur);
			if (ch == ' ' || (ch >= '0' && ch <= '9')){ // отдаем только пробелы и цифры
				return ch;
			}
		}
		
		return traits_type::eof();			
	}
public:
	numfilterbuf(istream &_in, ostream &_out)
		: in(&_in), out(&_out), cur(traits_type::eof())
	{}
};

int main(int argc, char **argv){
	const char str1[] = "In 4 bytes contains 32 bits";
	const char str2[] = "Unix time starts from Jan 1, 1970";
	istringstream str(str1);
	
	numfilterbuf buf(str, cout); // читать из stringstream, выводить в консоль
	iostream numfilter(&buf); // таким образом обходимся без реализации своего наследника iostream
	
	string val;
	getline(numfilter, val);
	numfilter.clear(); // сбросить невалидное состояние после EOF в процессе чтения из stringstream
	
	cout << "Original: '" << str1 << "'" << endl;
	cout << "Read from numfilter: '" << val << "'" << endl;
	
	cout << "Original: '" << str2 << "'" << endl;
	cout << "Written to numfilter: '";
	numfilter << str2;
	cout << "'" << endl;
	
	return 0;
}
