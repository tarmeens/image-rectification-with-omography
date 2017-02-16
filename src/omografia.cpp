#define _CRT_SECURE_NO_WARNINGS
#include "omografia.h"

omografia::omografia(Mat img) {
	immagine = img.clone();
	formato = FORMATO_A4;
	init = false;
}

omografia::omografia(Mat img, formatoImmagine fimg) {
	immagine = img.clone();
	if (fimg == FORMATO16_9 || fimg == FORMATO_A4)
		formato = fimg;
	else
		formato = FORMATO_A4;
	init = false;
}

omografia::omografia(Mat img, char *nome, formatoImmagine fimg) {
	immagine = img.clone();
	if (fimg == FORMATO16_9 || fimg == FORMATO_A4)
		formato = fimg;
	else
		formato = FORMATO_A4;
	strncpy(nomeImmagine, nome, 255);
	nomeImmagine[255] = 0;
	init = false;
}

void omografia::getNomeImmagine(char *s) {
	strcpy(s, nomeImmagine);
}


void omografia::formatoA4() {
	formato = FORMATO_A4;
}

void omografia::formato16_9(){
	formato = FORMATO16_9;
}

// Ottenere finestra di proporzioni A4. La lunghezza più lunga deve rimanere tale e le altre stirate. Formato 4:3 
void omografia::inizializzaPuntiA4(void) {
	if (init)
		for (int i = 0; i < 4;i++) pts_dst.pop_back();
	int width = immagine.size().width / coeff_rid;
	int height = width * ((double)297 / 210);
	pts_dst.push_back(Point2f(0, 0));
	pts_dst.push_back(Point2f(width, 0));
	pts_dst.push_back(Point2f(width, height));
	pts_dst.push_back(Point2f(0, height));
	init = true;
}

// Ottenere finestra di proporzioni 16:9. La lunghezza più lunga deve rimanere tale e le altre stirate. Formato 16:9
void omografia::inizializzaPunti16_9(void) {
	if (init) 
		for (int i = 0; i < 4; i++) pts_dst.pop_back();
	int width = immagine.size().width / coeff_rid;
	int height = width * ((double)9 / 16);
	pts_dst.push_back(Point2f(0, 0));
	pts_dst.push_back(Point2f(width, 0));
	pts_dst.push_back(Point2f(width, height));
	pts_dst.push_back(Point2f(0, height));
	init = true;
}

/*
 *	@brief  La funzione calcola la matrice che effettua l'omografia dell'immagine.
 *	@param1 src è il vettore dei 4 punti dell'immagine sorgente in input.
 *	@param2 dst è il vettore dei 4 punti dell'immagine desiderati in output.
 *  @ret	Ritorna la matrice dell'omografia. Se non vengono dati 4 punti, ritorna una matrice vuota.
 */
Mat omografia::trovaMatriceOmografia(vector<Point2f> src, vector<Point2f> dst)
{
	Mat res;
	// Valori double su un canale
	Mat A(8, 8, CV_64FC1); 
	Mat b(8, 1, CV_64FC1);
	Mat h(3, 3, CV_64FC1);
	// Se non inserisco 4 punti in in e out ritorno una matrice vuota.
	/*if (src.size != 4 || dst.size != 4) {
		return Mat::zeros(res.rows, res.cols, CV_8UC3); 
	}*/
	// Riempo la matrice dei coefficienti
	for (int i = 0; i < 4; i++) 
	{
		A.at<double>(0 + 2 * i, 0) = -1 * src.at(0 + i).x;
		A.at<double>(0 + 2 * i, 1) = -1 * src.at(0 + i).y;
		A.at<double>(0 + 2 * i, 2) = -1;
		A.at<double>(0 + 2 * i, 3) = 0;
		A.at<double>(0 + 2 * i, 4) = 0;
		A.at<double>(0 + 2 * i, 5) = 0;
		A.at<double>(0 + 2 * i, 6) = src.at(0 + i).x * dst.at(0 + i).x;
		A.at<double>(0 + 2 * i, 7) = src.at(0 + i).y * dst.at(0 + i).x;

		A.at<double>(1 + 2 * i, 0) = 0;
		A.at<double>(1 + 2 * i, 1) = 0;
		A.at<double>(1 + 2 * i, 2) = 0;
		A.at<double>(1 + 2 * i, 3) = -1 * src.at(0 + i).x;
		A.at<double>(1 + 2 * i, 4) = -1 * src.at(0 + i).y;
		A.at<double>(1 + 2 * i, 5) = -1;
		A.at<double>(1 + 2 * i, 6) = src.at(0 + i).x * dst.at(0 + i).y;
		A.at<double>(1 + 2 * i, 7) = src.at(0 + i).y * dst.at(0 + i).y;
	}
	// Riempo il vettore dei termini noti.
	for (int i = 0; i < 4; i++) 
	{
		b.at<double>(0 + 2 * i, 0) = -1 * dst.at(i).x;
		b.at<double>(1 + 2 * i, 0) = -1 * dst.at(i).y;
	}
	/* Risolvo il sistema */
	solve(A, b, res, DECOMP_LU); 
	/* Creazione matrice h di trasformazione dato il vettore di soluzioni */
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
		{
			if (i == 2 && j == 2) h.at<double>(i, j) = 1;   // FATTORE DI SCALA (H33). DI DEFAULT = 1. Ingrandisce e rimpicciolisce l'immagine.
			else h.at<double>(i, j) = res.at<double>(3 * i + j);
		}
	return h;
}

/*
 *	@brief  Effettua l'interpolazione bilineare dell'immagine.
 *	@param1 ch è la matrice che contiene il canale di un colore BGR.
 *	@param2 x posizione della coordinata x da interpolare.
 *  @param3 y posizione della coordinata y da interpolare.
 *  @ret	Ritorna il valore del colore BGR interpolato. 
 */
uchar omografia::interpolazioneBilineare(Mat ch, double x, double y) {
	int qx1, qx2;
	int qy1, qy2;
	// Coordinate dei 4 angoli adiacenti
	qx1 = x;		qy1 = y;
	qx2 = x + 1;	qy2 = y + 1;
	// Intensità canale pixel * coefficiente di distanza su x *  coefficiente di distanza su y 
	return ch.at<uchar>(qy1, qx1) * (qx2 - x) * (qy2 - y) + 
		ch.at<uchar>(qy1, qx2) * (x - qx1) * (qy2 - y) +
		ch.at<uchar>(qy2, qx1) * (qx2 - x) * (y - qy1) +
		ch.at<uchar>(qy2, qx2) * (x - qx1) * (y - qy1);
}

/*
 *  @brief	Effettua la traslazione dei punti secondo la matrice dell'omografia. Applica l'interpolazione bilineare per risolvere la posizione dei punti.
 *	@param1	srg è la matrice che contiene l'immagine da traslare.
 *	@param2 h è la matrice dell'omografia.
 *	@ret Ritorna l'immagine traslata.
 */
Mat omografia::traslaPuntiInterpolazioneBilineare(Mat src, Mat h)
{
	Mat res;
	res = src.clone();
	// Unsigned int 8 bit x 3 canali colore
	res = Mat::zeros(res.rows, res.cols, CV_8UC3); 
	Mat hinv = h.inv();
	// Canali con le matrici dei colori. Segue l'ordine BGR.
	vector<Mat> channelsSrc(3);
	vector<Mat> channelsRes(3);
	// Separo i canali
	split(src, channelsSrc);
	split(res, channelsRes);
	for (int i = 0; i < res.rows; i++)
	{
		for (int j = 0; j < res.cols; j++)
		{
			Mat vsrc(3, 1, CV_64FC1);
			Mat vres(3, 1, CV_64FC1);
			vres.at<double>(0, 0) = j;
			vres.at<double>(1, 0) = i;
			vres.at<double>(2, 0) = 1;
			vsrc = hinv * vres;
			double ni, nj;
			// DIVIDO PER IL FATTORE DI SCALA -> COORDINATE OMOGENEE
			nj = vsrc.at<double>(0, 0) / vsrc.at<double>(2, 0);  
			ni = vsrc.at<double>(1, 0) / vsrc.at<double>(2, 0);
			if ((nj > 0 && nj < (src.cols - 2)) && (ni > 0 && ni < (src.rows - 2)))
				for (int k = 0; k < 3;k++)
					channelsRes[k].at<uchar>(i, j) = interpolazioneBilineare(channelsSrc[k], nj, ni);
		}
	}
	// Ricreo l'immagine
	merge(channelsRes, res);
	return res;
}

/*
 *  @brief	Effettua la traslazione dei punti secondo la matrice dell'omografia. I punti vengono approssimati alla posizione più vicina.
 *	@param1	srg è la matrice che contiene l'immagine da traslare.
 *	@param2 h è la matrice dell'omografia.
 *	@ret Ritorna l'immagine traslata.
 */
Mat omografia::traslaPunti(Mat srg, Mat h)
{
	Mat res;
	res = srg.clone();
	// Unsigned int 8 bit x 3 canali colore
	res = Mat::zeros(res.rows, res.cols, CV_8UC3); 
	Mat hinv = h.inv();
	for (int i = 0; i < res.rows; i++)
	{
		for (int j = 0; j < res.cols; j++)
		{
			Mat vsrc(3, 1, CV_64FC1);
			Mat vres(3, 1, CV_64FC1);
			vres.at<double>(0, 0) = j;
			vres.at<double>(1, 0) = i;
			vres.at<double>(2, 0) = 1;
			vsrc = hinv * vres;
			int ni, nj;
			// DIVIDO PER IL FATTORE DI SCALA -> COORDINATE OMOGENEE
			// Arrotondamento dei risultati all'intero più vicino.
			nj = round(vsrc.at<double>(0, 0) / vsrc.at<double>(2, 0));  
			ni = round(vsrc.at<double>(1, 0) / vsrc.at<double>(2, 0));
			if ((nj > 0 && nj < (srg.cols - 1)) && (ni > 0 && ni< (srg.rows - 1)))
			{
				res.at<Vec3b>(i, j) = srg.at<Vec3b>(ni, nj);
			}
		}
	}
	return res;
}

/*
 *  @brief	Effettua l'omografia dell'immagine senza interpolazione.
 *	@param1	pts_src è il vettore dei punti sorgente.
 *	@ret Ritorna l'immagine elaborata.
 */
Mat omografia::elaboraImmagineConApprossimazione(vector<Point2f> pts_src) {
	Mat in_image;
	cout << "Elaborazione in corso..." << endl;
	if (formato == FORMATO_A4) inizializzaPuntiA4();
	if (formato == FORMATO16_9) inizializzaPunti16_9();
	in_image = immagine.clone();
	Mat h = trovaMatriceOmografia(pts_src, pts_dst);
	Mat out_image = traslaPunti(in_image, h);
	Rect ROI_crop(pts_dst.at(0).x, pts_dst.at(0).y, pts_dst.at(2).x - pts_dst.at(0).x, pts_dst.at(2).y - pts_dst.at(0).y);
	cout << "Elaborazione completata." << endl;
	return out_image(ROI_crop);
}

/*
 *  @brief	Effettua l'omografia dell'immagine.
 *	@param1 pts_src è il vettore dei punti sorgente.
 *	@ret Ritorna l'immagine elaborata.
 */
Mat omografia::elaboraImmagineConInterpolazione(vector<Point2f> pts_src) {
	Mat in_image;
	cout << "Elaborazione in corso..." << endl;
	if (formato == FORMATO_A4) inizializzaPuntiA4();
	if (formato == FORMATO16_9) inizializzaPunti16_9();
	in_image = immagine.clone();
	Mat out_image = immagine.clone();
	Mat h = trovaMatriceOmografia(pts_src, pts_dst);
	out_image = traslaPuntiInterpolazioneBilineare(in_image, h);
	Rect ROI_crop(pts_dst.at(0).x, pts_dst.at(0).y, pts_dst.at(2).x - pts_dst.at(0).x, pts_dst.at(2).y - pts_dst.at(0).y);
	cout << "Elaborazione completata." << endl;
	return out_image(ROI_crop);
}