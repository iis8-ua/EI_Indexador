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

IndexadorHash::IndexadorHash(const string& directorioIndexacion){
    ficheroStopWords = "";
    directorioIndice = "";
    tipoStemmer = 0;
    almacenarPosTerm = false;
    pregunta = "";

    RecuperarIndexacion(directorioIndexacion);
}

IndexadorHash::IndexadorHash(const IndexadorHash& t){
    indice = t.indice;
    indiceDocs = t.indiceDocs;
    informacionColeccionDocs = t.informacionColeccionDocs;
    pregunta = t.pregunta;
    indicePregunta = t.indicePregunta;
    infPregunta = t.infPregunta;
    stopWords = t.stopWords;
    ficheroStopWords = t.ficheroStopWords;
    tok = t.tok;
    directorioIndice = t.directorioIndice;
    tipoStemmer = t.tipoStemmer;
    almacenarPosTerm = t.almacenarPosTerm;
}

IndexadorHash::~IndexadorHash(){
    indice.clear();
    indiceDocs.clear();
    indicePregunta.clear();
    stopWords.clear();
    pregunta.clear();
    ficheroStopWords.clear();
    directorioIndice.clear();
}

IndexadorHash& IndexadorHash::operator= (const IndexadorHash& t){
    if (this != &t) {
        indice = t.indice;
        indiceDocs = t.indiceDocs;
        informacionColeccionDocs = t.informacionColeccionDocs;
        pregunta = t.pregunta;
        indicePregunta = t.indicePregunta;
        infPregunta = t.infPregunta;
        stopWords = t.stopWords;
        ficheroStopWords = t.ficheroStopWords;
        tok = t.tok;
        directorioIndice = t.directorioIndice;
        tipoStemmer = t.tipoStemmer;
        almacenarPosTerm = t.almacenarPosTerm;
    }
    return *this;
}

int IndexadorHash::NumPalIndexadas() const {
    return indice.size();
}

string IndexadorHash::DevolverFichPalParada() const {
    return ficheroStopWords;
}

int IndexadorHash::NumPalParada() const {
    return stopWords.size();
}

string IndexadorHash::DevolverDelimitadores() const {
    return tok.DelimitadoresPalabra();
}

bool IndexadorHash::DevolverCasosEspeciales() const {
    return const_cast<Tokenizador&>(tok).CasosEspeciales();
}

bool IndexadorHash::DevolverPasarAminuscSinAcentos() const {
    return const_cast<Tokenizador&>(tok).PasarAminuscSinAcentos();
}

bool IndexadorHash::DevolverAlmacenarPosTerm() const {
    return almacenarPosTerm;
}

string IndexadorHash::DevolverDirIndice() const {
    return directorioIndice;
}

int IndexadorHash::DevolverTipoStemming() const {
    return tipoStemmer;
}

void IndexadorHash::ListarPalParada() const {
    for (unordered_set<string>::const_iterator it = stopWords.begin(); it != stopWords.end(); ++it) {
        cout << *it << endl;
    }
}

void IndexadorHash::ListarInfColeccDocs() const {
    cout << informacionColeccionDocs << endl;
}

void IndexadorHash::ListarTerminos() const {
    for (unordered_map<string, InformacionTermino>::const_iterator it = indice.begin(); it != indice.end(); ++it) {
        cout << it->first << '\t' << it->second << endl;
    }
}

bool IndexadorHash::ListarTerminos(const string& nomDoc) const {
    unordered_map<string, InfDoc>::const_iterator itDoc = indiceDocs.find(nomDoc);
    if (itDoc == indiceDocs.end()) {
        return false;
    }
    int idDelDoc = itDoc->second.idDoc;

    for (unordered_map<string, InformacionTermino>::const_iterator itTerm = indice.begin(); itTerm != indice.end(); ++itTerm) {
        if (itTerm->second.l_docs.find(idDelDoc) != itTerm->second.l_docs.end()) {
            cout << itTerm->first << '\t' << itTerm->second << endl;
        }
    }
    return true;
}

void IndexadorHash::ListarDocs() const {
    for (unordered_map<string, InfDoc>::const_iterator it = indiceDocs.find(indiceDocs.begin()->first); it != indiceDocs.end(); ++it) {
        cout << it->first << '\t' << it->second << endl;
    }
}

bool IndexadorHash::ListarDocs(const string& nomDoc) const {
    unordered_map<string, InfDoc>::const_iterator it = indiceDocs.find(nomDoc);
    if (it != indiceDocs.end()) {
        cout << it->first << '\t' << it->second << endl;
        return true;
    }
    return false;
}

void IndexadorHash::VaciarIndiceDocs() {
    indice.clear();
    indiceDocs.clear();
    informacionColeccionDocs = InfColeccionDocs();
}

void IndexadorHash::VaciarIndicePreg() {
    pregunta.clear();
    indicePregunta.clear();
    infPregunta = InformacionPregunta();
}

bool IndexadorHash::Existe(const string& word) const {
    list<string> tokens;
    tok.Tokenizar(word, tokens);

    if (tokens.empty()) {
        return false;
    }

    string termino = tokens.front();

    if (tipoStemmer != 0) {
        stemmerPorter sp;
        sp.stemmer(termino, tipoStemmer);
    }

    return indice.find(termino) != indice.end();
}

bool IndexadorHash::Devuelve(const string& word, InformacionTermino& inf) const {
    list<string> tokens;
    tok.Tokenizar(word, tokens);

    if (tokens.empty()) {
        inf = InformacionTermino();
        return false;
    }

    string termino = tokens.front();

    if (tipoStemmer != 0) {
        stemmerPorter sp;
        sp.stemmer(termino, tipoStemmer);
    }

    unordered_map<string, InformacionTermino>::const_iterator it = indice.find(termino);

    if (it != indice.end()) {
        inf = it->second;
        return true;
    }
    else {
        inf = InformacionTermino();
        return false;
    }
}

bool IndexadorHash::Devuelve(const string& word, const string& nomDoc, InfTermDoc& InfDoc) const {
    unordered_map<string, class InfDoc>::const_iterator itDoc = indiceDocs.find(nomDoc);

    if (itDoc == indiceDocs.end()) {
        InfDoc = InfTermDoc();
        return false;
    }

    int idDelDoc = itDoc->second.idDoc;

    list<string> tokens;
    const_cast<Tokenizador&>(tok).Tokenizar(word, tokens);

    if (tokens.empty()) {
        InfDoc = InfTermDoc();
        return false;
    }

    string termino = tokens.front();

    if (tipoStemmer != 0) {
        stemmerPorter sp;
        sp.stemmer(termino, tipoStemmer);
    }

    unordered_map<string, InformacionTermino>::const_iterator itTerm = indice.find(termino);

    if (itTerm != indice.end()) {
        unordered_map<int, InfTermDoc>::const_iterator itAparicion = itTerm->second.l_docs.find(idDelDoc);
        if (itAparicion != itTerm->second.l_docs.end()) {
            InfDoc = itAparicion->second;
            return true;
        }
    }

    InfDoc = InfTermDoc();
    return false;
}

bool IndexadorHash::DevuelvePregunta(string& preg) const {
    if (indicePregunta.empty()) {
        return false;
    }
    preg = pregunta;
    return true;
}

bool IndexadorHash::DevuelvePregunta(InformacionPregunta& inf) const {
    if (indicePregunta.empty()) {
        inf = InformacionPregunta();
        return false;
    }
    inf = infPregunta;
    return true;
}

bool IndexadorHash::DevuelvePregunta(const string& word, InformacionTerminoPregunta& inf) const {
}

bool IndexadorHash::RecuperarIndexacion(const string& directorioIndexacion) {
}

bool IndexadorHash::IndexarPregunta(const string& preg) {
}

bool IndexadorHash::BorraDoc(const string& nomDoc) {

}

bool IndexadorHash::Indexar(const string& ficheroDocumentos) {

}

bool IndexadorHash::IndexarDirectorio(const string& dirAIndexar) {
}



