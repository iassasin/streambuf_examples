#include <iostream>
#include <string>
#include <vector>
#include <cstdio>
#include <cstdlib>

using namespace std;

class cfilebuf : public streambuf {
private:
	vector<char_type> buffer;
	FILE *file;
protected:
	virtual int underflow() override {
		if (!file)
			return traits_type::eof();
			
		if (gptr() < egptr()) //если буфер не пуст, вернем текущий символ
			return *gptr();
		
		char_type *start = eback();
		//читаем не больше символов, чем вмещает буфер
		size_t rd = fread(start, sizeof(char_type), buffer.size(), file);
		//указываем размер буфера не больше, чем было считано символов
		setg(start, start, start + rd);
		
		return rd > 0 ? *gptr() : traits_type::eof();
	}
public:
	cfilebuf(size_t _bufsize)
		: buffer(_bufsize), file(nullptr)
	{
		char_type *start = buffer.data();
		char_type *end = start + buffer.size();
		setg(start, end, end); //устанавливаем eback = start, gptr = end, egptr = end
		//т.к. gptr == egptr, буфер по факту пуст и будет заполнен при первой попытке чтения
	}
	
	~cfilebuf(){
		close();
	}
	
	bool open(string fn){
		close();
		file = fopen(fn.c_str(), "r");
		return file != nullptr;
	}
	
	void close(){
		if (file){
			fclose(file);
			file = nullptr;
		}
	}
};

int main(int argc, char **argv){
	cfilebuf buf(10);
	istream in(&buf);
	string line;
	
	buf.open("file.txt");
	while (getline(in, line)){
		cout << line << endl;
	}
	
	return 0;
}
