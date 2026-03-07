#include "indexadorHash.h"

using namespace std;

IndexadorHash::IndexadorHash(const string& fichStopWords, const string& delimitadores, const bool& detectComp, const bool& minuscSinAcentos, const string& dirIndice, const int& tStemmer, const bool& almPosTerm){
    ficheroStopWords = fichStopWords;
    directorioIndice = dirIndice;
    tipoStemmer = tStemmer;
    almacenarPosTerm = almPosTerm;

    tok.DelimitadoresPalabra(delimitadores);
    tok.CasosEspeciales(detectComp);
    tok.PasarAminuscSinAcentos(minuscSinAcentos);

    CargarStopWords(fichStopWords);
}

void IndexadorHash::CargarStopWords(const string& fichStopWords) {
    ifstream archivo(fichStopWords.c_str());
    if (!archivo) {
        cerr << "ERROR: No existe el archivo de palabras de parada: " << fichStopWords << endl;
        return;
    }

    stopWords.clear();
    string linea;
    stemmerPorter sp;

    while (getline(archivo, linea)) {
        if (!linea.empty()) {
            list<string> tokens;
            tok.Tokenizar(linea, tokens);

            for (list<string>::iterator it = tokens.begin(); it != tokens.end(); ++it) {
                if (tipoStemmer != 0) {
                    sp.stemmer(*it, tipoStemmer);
                }
                stopWords.insert(*it);
            }
        }
    }
    archivo.close();
}