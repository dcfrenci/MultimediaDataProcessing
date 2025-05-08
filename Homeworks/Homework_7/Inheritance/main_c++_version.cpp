#define _CRT_SECURE_NO_WARNINGS

#include "canvas.h"
#include <string>
#include <memory>

#include <crtdbg.h>

struct shape { // abstract class
	int x_, y_;
	char c_;
	std::string name_;

	shape(int x, int y, char c) 
		: x_(x), y_(y), c_(c)
		, name_("shape")
	{
	}
	virtual ~shape() = default;
	virtual void draw(canvas& c) const = 0; // pure virtual function
	void setname(std::string name) {
		name_ = std::move(name);
	}
};

struct point : public shape {
	std::unique_ptr<int> what_; // more resources

	point(int x, int y, char c) : shape(x, y, c) {
		what_ = std::make_unique<int>(0x00a0ccca);
		setname("point");
	}
	void draw(canvas& c) const override {
		c.set(x_, y_, c_);
	}
};

struct line : public shape {
	int x1_, y1_;

	line(int x, int y, int x1, int y1, char c) 
		: shape(x, y, c), x1_(x1), y1_(y1)
	{
		setname("line");
	}
	void draw(canvas& c) const override {
		c.line(x_, y_, x1_, y1_, c_);
	}
};

struct rectangle: public shape {
	int x1_, y1_;

	rectangle(int x, int y, int x1, int y1, char c) 
		: shape(x, y, c), x1_(x1), y1_(y1) 
	{
		setname("rectangle");
	}
	void draw(canvas& c) const override {
		c.rectangle(x_, y_, x1_, y1_, c_);
	}
};

struct circle : public shape {
	int r_;

	circle(int x, int y, int r, char c) 
		: shape(x, y, c), r_(r)
	{
		setname("circle");
	}
	void draw(canvas& c) const override {
		c.circle(x_, y_, r_, c_);
	}
};


int main(void)
{
	using namespace std;
	{
		canvas c(80, 25);

		vector<unique_ptr<shape>> shapes;

		shapes.push_back(make_unique<rectangle>(0, 0, 79, 24, '*'));
		shapes.push_back(make_unique<point>(5, 15, '?'));
		shapes.push_back(make_unique<circle>(10, 10, 4, '@'));
		shapes.push_back(make_unique<circle>(70, 10, 4, '@'));
		shapes.push_back(make_unique<line>(40, 15, 40, 20, '|'));

		for (const auto& s : shapes) {
			s->draw(c);
		}

		c.out(stdout);
	}
	_CrtDumpMemoryLeaks();
}
