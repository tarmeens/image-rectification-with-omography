#pragma once

#include <iostream>
#include "opencv2/opencv.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include <string.h>

/* Dichiarazione dei namespace necessari per OpenCV*/
using namespace std;
using namespace cv;

/* Enumerazione formato immagine */
enum formatoImmagine {
	FORMATO_A4,
	FORMATO16_9
};

class omografia
{	
public:
	omografia(Mat img);
	omografia(Mat img,formatoImmagine fimg);
	omografia(Mat img, char *nome, formatoImmagine fimg);
	void getNomeImmagine(char *s);
	void formatoA4();
	void formato16_9();
	Mat elaboraImmagineConApprossimazione(vector<Point2f> pts_src);
	Mat elaboraImmagineConInterpolazione(vector<Point2f> pts_src);
private:
	char nomeImmagine[256];
	Mat immagine;
	formatoImmagine formato;
	const float coeff_rid = 2;
	vector<Point2f> pts_dst;
	bool init;

	void inizializzaPunti16_9();
	void inizializzaPuntiA4();
	Mat trovaMatriceOmografia(vector<Point2f> src, vector<Point2f> dst);
	static uchar interpolazioneBilineare(Mat ch, double x, double y);
	Mat traslaPuntiInterpolazioneBilineare(Mat src, Mat h);
	Mat traslaPunti(Mat srg, Mat h);
};