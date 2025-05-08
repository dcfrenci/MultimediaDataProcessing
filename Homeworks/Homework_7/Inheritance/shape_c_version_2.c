#define _CRT_SECURE_NO_WARNINGS
#include <stdbool.h>
#include <stdlib.h>

#include "canvas_c_version.h"
#include "string.h"

#include "crtdbg.h"

/*
type_of_object parameter_1 parameter_2 ... parameter_n

rectangle 0 0 79 24 *
point 5 15 ?
circle 10 10 4 @
circle 70 10 4 @
line 40 15 40 20 |
*/
/*
void draw_file(FILE *f, canvas_c_version *c)
{
	char type[30];
	while (fscanf(f, "%29s", type) == 1) {
		if (strcmp(type, "point") == 0) {
			int x, y;
			char ch;
			fscanf(f, "%d%d %c", &x, &y, &ch);
			canvas_set(c, x, y, ch);
		}
		else if (strcmp(type, "line") == 0) {
			int x0, y0, x1, y1;
			char ch;
			fscanf(f, "%d%d%d%d %c", &x0, &y0, &x1, &y1, &ch);
			canvas_line(c, x0, y0, x1, y1, ch);
		}
		else if (strcmp(type, "rectangle") == 0) {
			int x0, y0, x1, y1;
			char ch;
			fscanf(f, "%d%d%d%d %c", &x0, &y0, &x1, &y1, &ch);
			canvas_rectangle(c, x0, y0, x1, y1, ch);
		}
		else if (strcmp(type, "circle") == 0) {
			int xm, ym, r;
			char ch;
			fscanf(f, "%d%d%d %c", &xm, &ym, &r, &ch);
			canvas_circle(c, xm, ym, r, ch);
		}
	}
}
*/

#define v(x) (void*)(x)

typedef void (*fnptr)(void*, ...);

typedef struct shape {
	int x_, y_;
	char c_;
	char* name_;
	fnptr* vtbl;
} shape;
void shape_draw(shape* this, canvas* c) {
	canvas_set(c, this->x_, this->y_, this->c_); // dummy
}
shape* shape_destroy(shape* this) {
	free(this->name_);
	return this;
}
fnptr shape_vtbl[] = { shape_draw, shape_destroy };
shape* shape_create(shape* this, int x, int y, char c) {
	this->x_ = x;
	this->y_ = y;
	this->c_ = c;
	this->name_ = _strdup("shape");
	this->vtbl = shape_vtbl;
	return this;
}
shape* new_shape(int x, int y, char c) {
	return shape_create(malloc(sizeof(shape)), x, y, c);
}
void delete_shape(shape* this) {
	this->vtbl[1](this);
	free(this);
}
void shape_setname(shape* this, const char* name) {
	free(this->name_);
	this->name_ = _strdup(name);
}

typedef struct point {
	shape base;
	int* what_; // more resources
} point;
void point_draw(point* this, canvas* c) {
	shape_draw(v(this), c);
}
point* point_destroy(point* this) {
	free(this->what_);
	shape_destroy(v(this));
	return this;
}
fnptr point_vtbl[] = { point_draw, point_destroy };
point* point_create(point* this, int x, int y, char c) {
	shape_create(v(this), x, y, c);
	this->base.vtbl = point_vtbl;
	this->what_ = malloc(sizeof(int));
	this->what_[0] = 0x00a0ccca;
	shape_setname(v(this), "point");
	return this;
}
point* new_point(int x, int y, char c) {
	return point_create(malloc(sizeof(point)), x, y, c);
}
void delete_point(point* this) {
	free(point_destroy(this));
}

typedef struct line {
	shape base;
	int x1_, y1_;
} line;
void line_draw(line* this, canvas* c) {
	canvas_line(c, this->base.x_, this->base.y_,
		this->x1_, this->y1_, this->base.c_);
}
line* line_destroy(line* this) {
	shape_destroy(v(this));
	return this;
}
fnptr line_vtbl[] = { line_draw, line_destroy };
line* line_create(line* this, int x, int y, int x1, int y1, char c) {
	shape_create(v(this), x, y, c);
	this->base.vtbl = line_vtbl;
	this->x1_ = x1;
	this->y1_ = y1;
	shape_setname(v(this), "line");
	return this;
}
line* new_line(int x, int y, int x1, int y1, char c) {
	return line_create(malloc(sizeof(line)), x, y, x1, y1, c);
}
void delete_line(line* this) {
	free(line_destroy(this));
}

typedef struct rectangle {
	shape base;
	int x1_, y1_;
} rectangle;
void rectangle_draw(rectangle* this, canvas* c) {
	canvas_rectangle(c, this->base.x_, this->base.y_,
		this->x1_, this->y1_, this->base.c_);
}
rectangle* rectangle_destroy(rectangle* this) {
	shape_destroy(v(this));
	return this;
}
fnptr rectangle_vtbl[] = { rectangle_draw, rectangle_destroy };
rectangle* rectangle_create(rectangle* this, int x, int y, int x1, int y1, char c) {
	shape_create(v(this), x, y, c);
	this->base.vtbl = rectangle_vtbl;
	this->x1_ = x1;
	this->y1_ = y1;
	shape_setname(v(this), "rectangle");
	return this;
}
rectangle* new_rectangle(int x, int y, int x1, int y1, char c) {
	return rectangle_create(malloc(sizeof(rectangle)), x, y, x1, y1, c);
}
void delete_rectangle(rectangle* this) {
	free(rectangle_destroy(this));
}

typedef struct circle {
	shape base;
	int r_;
} circle;
void circle_draw(circle* this, canvas* c) {
	canvas_circle(c, this->base.x_, this->base.y_, this->r_, this->base.c_);
}
circle* circle_destroy(circle* this) {
	shape_destroy(v(this));
	return this;
}
fnptr circle_vtbl[] = { circle_draw, circle_destroy };
circle* circle_create(circle* this, int x, int y, int r, char c) {
	shape_create(v(this), x, y, c);
	this->base.vtbl = circle_vtbl;
	this->r_ = r;
	shape_setname(v(this), "circle");
	return this;
}
circle* new_circle(int x, int y, int r, char c) {
	return circle_create(malloc(sizeof(circle)), x, y, r, c);
}
void delete_circle(circle* this) {
	free(circle_destroy(this));
}





int main(void)
{
	canvas* c = new_canvas(80, 25);

	//draw_file(stdin, c);
	//rectangle 0 0 79 24 *
	//point 5 15 ?
	//circle 10 10 4 @
	//circle 70 10 4 @
	//line 40 15 40 20 |
	rectangle* r1 = new_rectangle(0, 0, 79, 24, '*');
	point* p1 = new_point(5, 15, '?');
	circle* c1 = new_circle(10, 10, 4, '@');
	circle* c2 = new_circle(70, 10, 4, '@');
	line* l1 = new_line(40, 15, 40, 20, '|');

	shape* shapes[] = { v(r1), v(p1), v(c1), v(c2), v(l1) };
	size_t nshapes = 5;

	for (size_t i = 0; i < nshapes; ++i) {
		shapes[i]->vtbl[0](shapes[i], c);
	}

	canvas_out(c, stdout);

	for (size_t i = 0; i < nshapes; ++i) {
		delete_shape(shapes[i]);
	}
	delete_canvas(c);
	_CrtDumpMemoryLeaks();
}