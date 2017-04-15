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
	streampos pos_base; //позиция в файле для eback

	streampos fill_buffer_from(streampos newpos, int dir = SEEK_SET) {
		if (!file || fseek(file, newpos, dir) == -1)
			return -1;
		//запоминаем текущую позицию в файле для eback
		long pos = ftell(file);
		if (pos < 0)
			return -1;
		pos_base = pos;

		char_type *start = eback();
		//читаем не больше символов, чем вмещает буфер
		size_t rd = fread(start, sizeof(char_type), buffer.size(), file);
		//указываем размер буфера не больше, чем было считано символов
		setg(start, start, start + rd);

		return rd > 0 && pos_base >= 0 ? pos_base : streampos(-1);
	}
protected:
	virtual int underflow() override {
		if (!file)
			return traits_type::eof();

		if (gptr() < egptr()) //если буфер не пуст, вернем текущий символ
			return *gptr();

		streampos pos;
		if (pos_base < 0) { //если буфер еще не заполнялся, заполняем с начала
			pos = fill_buffer_from(0);
		}
		else { //иначе заполняем со следующего несчитанного символа
			pos = fill_buffer_from(pos_base + egptr() - eback());
		}

		return pos != streampos(-1) ? *gptr() : traits_type::eof();
	}

	//второй аргумент в нашем случае всегда содержит ios_base::in
	//однако в общем случае может содержать и ios_base::out и даже сразу оба (побитовое ИЛИ)
	virtual streampos seekpos(streampos sp, ios_base::openmode which) override {
		if (!(which & ios_base::in))
			return streampos(-1);
		return fill_buffer_from(sp);
	}

	//обработка трех вариантов позиционирования: с начала, с текущей позиции и с конца
	virtual streampos seekoff(streamoff off, ios_base::seekdir way, ios_base::openmode which) override {
		if (!(which & ios_base::in))
			return streampos(-1);

		switch (way) {
		default:
		case ios_base::beg: return fill_buffer_from(off, SEEK_SET);
		case ios_base::cur: return fill_buffer_from(pos_base + gptr() - eback() + off); //учитываем позицию от начала в нашем буфере
		case ios_base::end: return fill_buffer_from(off, SEEK_END);
		}
	}

	virtual int pbackfail(int c) override {
		//когда gptr > eback, значит в буфере есть символ на нужной позиции,
		//но он не совпал с переданным, запрещаем
		if (pos_base <= 0 || gptr() > eback())
			return traits_type::eof();

		//загружаем в буфер данные, начиная с предыдущего символа	
		if (fill_buffer_from(pos_base - streampos(1L)) == streampos(-1))
			return traits_type::eof();

		if (*gptr() != c) {
			gbump(1); //возвращаемся назад, неудачная операция
			return traits_type::eof();
		}

		return *gptr();
	}
public:
	cfilebuf(size_t _bufsize)
		: buffer(_bufsize), file(nullptr), pos_base(-1)
	{
		char_type *start = buffer.data();
		char_type *end = start + buffer.size();
		setg(start, end, end); //устанавливаем eback = start, gptr = end, egptr = end
	}

	~cfilebuf() {
		close();
	}

	bool open(string fn) {
		close();
		file = fopen(fn.c_str(), "r");
		return file != nullptr;
	}

	void close() {
		if (file) {
			fclose(file);
			file = nullptr;
		}
	}
};

void read_to_end(istream &in) {
	string line;
	while (getline(in, line)) {
		cout << line << endl;
	}
}

int main(int argc, char **argv) {
	cfilebuf buf(10);
	istream in(&buf);
	buf.open("file.txt");

	read_to_end(in);
	in.clear(); //очистить невалидное состояние после конца файла

	cout << endl << endl << "Read last 6 symbols:" << endl;
	in.seekg(-5, ios_base::end); //передвинем позицию так, чтобы можно было считать 5 последних символов
	in.seekg(-1, ios_base::cur); //а лучше 6, чтобы слово целиком влезло :)
	read_to_end(in);
	in.clear();

	cout << endl << endl << "Read all again:" << endl;
	in.seekg(0);
	read_to_end(in);
	in.clear();

	in.seekg(2); //заставляем наш буфер начинаться с 3-го символа в файле (чтобы в буфере не было первых 2-ух)
	in.get();
	in.putback('b');
	in.putback('a'); //без pbackfail() этот код не сработал бы и привел бы поток в невалидное состояние
	in.putback('H');

	string word;
	in >> word;
	cout << endl << endl << "Read word after putback(): " << word << endl;

	return 0;
}
