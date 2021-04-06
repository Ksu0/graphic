
#include <fstream>
#include <cmath>
#include <cstdint>
#include <vector>
#include <iostream>
#include <array>
#include <cassert>


#pragma pack(push, 1)
struct TGA_Header
{
	uint8_t id_len;			// длина идентификатора
	uint8_t pal_type;		// тип палитры
	uint8_t img_type;		// тип изображения
	uint8_t pal_desc[5];	// описание палитры
	uint16_t x_pos;			// положение по оси X
	uint16_t y_pos;			// положение по оси Y
	uint16_t width;			// ширина
	uint16_t height;		// высота
	uint8_t depth;			// глубина цвета
	uint8_t img_desc;		// опис. изображения
};
#pragma pack(pop)

constexpr uint16_t IMG_WIDTH = 1920;
constexpr uint16_t IMG_HEIGHT = 1080;
constexpr uint32_t COL_BACKGROUND = 0xff994db5;
constexpr uint32_t COL_FOREGROUND = 0xffcfcfcf;
constexpr double rx = 15., ry = 15., rz = 300.,
		xmax = 30., xmin = -30., ymax = 30., ymin = -30.,
		step1 = 0.5, step2 = 0.01;

double sinc(double x)
{
	if (x == 0) return 1.;
	return sin(x) / x;
}

double my_evil_function(double x, double y)
{
	return sinc(hypot(x, y));
}

void pic(std::vector<uint32_t>& pix, int x, int y, uint32_t c, std::vector<int>& h)
{
	if (x < 0 or x >= IMG_WIDTH)
		return;
	if (y > h[x])
		return;
	h[x]=y;
	if(y < 0)
		return;
	assert(y * IMG_WIDTH + x >= 0);
	assert(y * IMG_WIDTH + x < IMG_WIDTH * IMG_HEIGHT);
	pix[y*IMG_WIDTH + x] = c;

}


int main()
{
	// построить график функции в некотором буфере
	std::vector<uint32_t> picture(IMG_WIDTH * IMG_HEIGHT);
	for (auto && p : picture) p = COL_BACKGROUND;

	int sx=0,sy=0;
	double z;
	std::fstream tga_file;
	std::vector<int> h_w;
	h_w.resize(IMG_WIDTH);

	for(auto && a:h_w)
		a=IMG_HEIGHT;

	//z = sinc(sqrt(x*x + y*y))
	for (double i = xmax; i >= xmin; i -= step1) {
		for (double j = ymax; j >= ymin; j -= step2) {
			z = my_evil_function(i, j);
			sx = int(IMG_WIDTH / 2 - rx * i * cos(M_PI / 6) + ry * j * cos(M_PI / 6));
			sy = int(IMG_HEIGHT / 2 + rx * i * sin(M_PI / 6) + ry * j * sin(M_PI / 6) - rz * z);
			pic(picture, sx, sy, COL_FOREGROUND,h_w);
		}
	}

	for (auto&& a : h_w)
		a = IMG_HEIGHT;

	for (double i = xmax; i >= xmin; i -= step2) {
		for (double j = ymax; j >= ymin; j -= step1) {
			z = my_evil_function(i, j);
			sx = int(IMG_WIDTH / 2 - rx * i * cos(M_PI / 6) + ry * j * cos(M_PI / 6));
			sy = int(IMG_HEIGHT / 2 + rx * i * sin(M_PI / 6) + ry * j * sin(M_PI / 6) - rz * z);
			pic(picture, sx, sy, COL_FOREGROUND,h_w);
		}
	}


	// записать построенное изображение в файл формата tga
	TGA_Header hdr {};
	hdr.width = IMG_WIDTH;
	hdr.height = IMG_HEIGHT;
	hdr.depth = 32;
	hdr.img_type = 2;
	hdr.img_desc = 0x28;

	tga_file.open("output.tga",std::ios::out|std::ios::binary);
	// записываем заголовок и данные картинки
	tga_file.write(reinterpret_cast<char*>(&hdr),sizeof(TGA_Header));
		tga_file.write(reinterpret_cast<char*>(&picture[0]),IMG_WIDTH*IMG_HEIGHT*4);
	// закрываем файл
	tga_file.close();

	return 0;
}


