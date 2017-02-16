#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include "opencv2/opencv.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "omografia.h"
#include <string.h>


using namespace std;
using namespace cv;


Mat src, img;
/* Contiene i 4 punti di partenza per calcolare l'omografia*/
vector<Point2f> pts_src;

char buffer[256]; // Attenzione al buffer overflow!!!!
char winName[30];
bool clicked = false;
char imgName[15];
char imgOutName[20];
int pointcount = 0;
bool afferrato = false;
int indice = 0;
const float coeff = 4;

/* Stampa la finestra con l'immagine */
void showImage() {
	imshow(winName, img);
}

/* Inserisce un nuovo punto nell'immagine, usata solo nella fase di iniziallizazione */
void disegnaPunti(int x, int y) {
		pts_src.push_back(Point2f(x, y));
		pointcount++;
}

/* Disegna le linee sullo schermo */
void disegnaLinee(int x, int y) {
	// Clono l'immagine dentro una nuova matrice
	img = src.clone();
	if (pointcount == 0) 
		return;
	if (pointcount == 1)
		line(img, pts_src[0], Point2f(x, y), Scalar(110, 220, 0), 2, 8);
	if (pointcount == 2) {
		line(img, pts_src[0], pts_src[1], Scalar(110, 220, 0), 2, 8);
		line(img, pts_src[1], Point2f(x, y), Scalar(110, 220, 0), 2, 8);
	}
	if (pointcount == 3) {
		line(img, pts_src[0], pts_src[1], Scalar(110, 220, 0), 2, 8);
		line(img, pts_src[1], pts_src[2], Scalar(110, 220, 0), 2, 8);
		line(img, pts_src[2], Point2f(x, y), Scalar(110, 220, 0), 2, 8);
	}
	if (pointcount == 4) {
		line(img, pts_src[0], pts_src[1], Scalar(110, 220, 0), 2, 8);
		line(img, pts_src[1], pts_src[2], Scalar(110, 220, 0), 2, 8);
		line(img, pts_src[2], pts_src[3], Scalar(110, 220, 0), 2, 8);
		line(img, pts_src[3], pts_src[0], Scalar(110, 220, 0), 2, 8);
	}
}

/* Inizializzazione dei punti nell'immagine */
void initPoints() {
	pointcount = 0;
	int oy = src.size().height / 2;
	int ox = src.size().width / 2;
	int offx = ox * 0.1;
	int offy = oy * 0.1;
	disegnaPunti(ox - offx, oy - offy);
	disegnaPunti(ox + offx, oy - offy);
	disegnaPunti(ox + offx, oy + offy);
	disegnaPunti(ox - offx, oy + offy);
	disegnaLinee(0, 0);
}


// Trovo il punto sorgente più vicino alla posizione del mouse
bool cercaPunti(int x, int y, Point2f *punto, int *indice) {
	const int scarto = 10;
	for (int i = 0; i < 4; i++) {
		if (pts_src[i].x - scarto < x && pts_src[i].x + scarto > x && pts_src[i].y + scarto > y && pts_src[i].y - scarto < y) {
			*punto = pts_src[i];
			*indice = i;
			return true;
		}
	}
	return false;
}

void onMouse(int event, int x, int y, int f, void*) {
	Point2f punto = NULL;
	switch (event) {
	case  CV_EVENT_LBUTTONDOWN:
			clicked = true;
			if(!afferrato) 
				afferrato = cercaPunti(x, y, &punto, &indice);
		break;

	case  CV_EVENT_MOUSEMOVE:
		if (clicked) {
			if (afferrato) {
				pts_src[indice].x = x;
				pts_src[indice].y = y;
			}
		}
		break;

	case  CV_EVENT_LBUTTONUP:
		clicked = false;
		afferrato = false;
		break;

	default:   break;
	}
	if (clicked) {
		if (afferrato) {
			pts_src[indice].x = x;
			pts_src[indice].y = y;
		}
	}
	disegnaLinee(x, y);
	showImage();
}

/* Resetta la posizione dei punti */
void reset() {
	if(pointcount > 0)
		for (; pointcount > 0; pointcount--) pts_src.pop_back();
	clicked = false;
	afferrato = false;
	indice = 0;
	initPoints();
	showImage();
}

void aiuto(void) {
	cout << "Aiuto:" << endl;
	cout << "Gli angoli della regione di interesse corrispondono a quelli della futura immagine generata." << endl;
	cout << "L'angolo in alto a sinistra sara' quindi quello in alto a sinistra nella nuova immagine, lo stesso vale per gli altri angoli." << endl;
	cout << "Bisogna quindi scegliere accuratamente la posizione degli angoli." << endl;
	cout << "Premendo 'n' e' possibile trasformare l'immagine senza farne l'omografia." << endl;
	cout << "Scegliere con 'd' e 'f' il funzionamento in modalita' diapositiva o foglio. Di default e' impostato su foglio" << endl << endl;
}

int main(int argc, char **argv)
{
	if (argc != 2) {
		cout << "ERRORE: numero di parametri errato. Formato: \"Elaborazione Diapositive\" nomeimmagine.jpg" << endl;
		return 0;
	}
	cout << "Applicazione pratica dell' omografia su fogli e diapositive." << endl;
	cout << "Creato da Mattia Martinelli per la Prova Finale 5 Luglio 2013 a Unimore." << endl << endl;

	cout << "Trascina gli angoli del rettangolo verde sugli angoli della regione da modificare" << endl;
	cout << "---> Premi 'Invio' per effettuare l'omografia dell'immagine " << endl;
	cout << "---> Premi 'n' per effettuare l'omografia dell'immagine senza l'interpolazione" << endl;
	cout << "---> Premi 'f' per generare un foglio A4" << endl;
	cout << "---> Premi 'd' per generare una diapositiva 16:9" << endl;
	cout << "---> Premi 'r' per resettare le posizioni dei punti" << endl;
	cout << "---> Premi 'a' per aiuto" << endl;
	cout << "---> Premi 'Esc' per uscire" << endl << endl;

	sprintf(imgName, argv[1]);
	sprintf(winName, "%s - Da modificare", argv[1]);
	//sprintf(imgName, "src2.jpg");
	//sprintf(winName, "debug");
	src = imread(imgName, 1);
	img = src.clone();  // Clono l'immagine dentro una nuova matrice
	namedWindow(winName, WINDOW_NORMAL);
	setMouseCallback(winName, onMouse, NULL);
	initPoints();
	showImage();
	omografia omo = omografia(src);
	/* Finestra di anteprima: dimensione di default A4 */
	Size size((img.size().width / coeff), (img.size().width / coeff) * ((float)297 / 210));
	while (1) {
		char c = waitKey();
		if (c == 13 && pointcount == 4) {
			Mat ant1;
			Mat result;
			result = omo.elaboraImmagineConInterpolazione(pts_src);
			strcpy(buffer, imgName);
			buffer[strlen(imgName) - 4] = 0; // tolgo l'estensione
			sprintf(imgOutName, "%s_new.jpg", buffer); 
			imwrite(imgOutName, result);  // Scrive l'immagine modificata su disco nel file imgName
			cout << "Nuova immagine " << imgOutName << " salvata! " << endl;
			resize(result, ant1, size);//resize image
			sprintf(buffer, "%s - Anteprima", imgOutName);
			imshow(buffer, ant1);
		}
		if ((c == 'N' || c == 'n') && pointcount == 4) {
			Mat ant2;
			Mat result2;
			result2 = omo.elaboraImmagineConApprossimazione(pts_src);
			strcpy(buffer, imgName);
			buffer[strlen(imgName) - 4] = 0; // tolgo l'estensione
			sprintf(imgOutName, "%s_newNI.jpg", buffer); 
			imwrite(imgOutName, result2);  // Scrive l'immagine modificata su disco nel file imgName
			cout << "Nuova immagine " << imgOutName << " salvata! (senza interpolazione) " << endl;
			resize(result2, ant2, size);//resize image
			sprintf(buffer, "%s - Anteprima", imgOutName);
			imshow(buffer, ant2);
		}
		if (c == 'f' || c == 'F')
		{
			cout << "Impostato formato A4" << endl;
			size.height = size.width * ((float)297 / 210);
			omo.formatoA4();
		}
		if (c == 'd' || c == 'D')
		{
			cout << "Impostato formato diapositiva 16:9" << endl;
			size.height = size.width * ((float)9 / 16);
			omo.formato16_9();
		}
		if (c == 'a' || c == 'A') { aiuto(); }
		if (c == 27)
		{	
			break;
		}
		if (c == 'r') { reset(); }

		showImage();
	}
	return 0;
}