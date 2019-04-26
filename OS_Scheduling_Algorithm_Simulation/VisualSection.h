#ifndef VISUAL_SECTION_H_
#define VISUAL_SECTION_H_

struct VisualSection
{
	int startX, startY;
	int width, height;

	VisualSection(int startX, int startY, int width, int height)
	{
		this->startX = startX;
		this->startY = startY;
		this->width = width;
		this->height = height;
	}
};
#endif // !VISUAL_SECTION_H_
