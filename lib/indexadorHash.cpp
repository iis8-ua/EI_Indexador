#include "indexadorHash.h"

using namespace std;

// ── Constructores y destructor ────────────────────────────────────────────────

IndexadorHash::IndexadorHash(const string& fichStopWords, const string& delimitadores, const bool& detectComp, const bool& minuscSinAcentos, const string& dirIndice, const int& tStemmer, const bool& almPosTerm){
    ficheroStopWords = fichStopWords;
    directorioIndice = dirIndice;
    tipoStemmer = tStemmer;
    almacenarPosTerm = almPosTerm;

    tok.DelimitadoresPalabra(delimitadores);
    tok.CasosEspeciales(detectComp);
    tok.PasarAminuscSinAcentos(minuscSinAcentos);

    CargarStopWords(fichStopWords);

    // Reserva inicial para evitar rehashes costosos durante la indexación.
    // Ajusta 1<<17 (131072) según el tamaño esperado del corpus.
    indice.reserve(1 << 17);
    indiceDocs.reserve(1024);
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

// ── Stop words ────────────────────────────────────────────────────────────────

void IndexadorHash::CargarStopWords(const string& fichStopWords) {
    ifstream archivo(fichStopWords.c_str());
    if (!archivo) {
        cerr << "ERROR: No existe el archivo de palabras de parada: " << fichStopWords << endl;
        return;
    }

    stopWords.clear();
    string linea;

    while (getline(archivo, linea)) {
        if (!linea.empty()) {
            list<string> tokens;
            tok.Tokenizar(linea, tokens);
            for (list<string>::iterator it = tokens.begin(); it != tokens.end(); ++it) {
                stopWords.insert(*it);
            }
        }
    }
    archivo.close();
}

// ── Getters ───────────────────────────────────────────────────────────────────

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

// ── Listar ────────────────────────────────────────────────────────────────────

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
    for (unordered_map<string, InfDoc>::const_iterator it = indiceDocs.begin(); it != indiceDocs.end(); ++it) {
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

// ── Vaciar ────────────────────────────────────────────────────────────────────

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

// ── PrepararTermino ───────────────────────────────────────────────────────────

bool IndexadorHash::PrepararTermino(const string& word, Tokenizador& tok, int tipoStemmer, string& termino) const {
    list<string> tokens;
    tok.Tokenizar(word, tokens);
    if (tokens.empty()) {
        return false;
    }

    termino = tokens.front();
    if (tipoStemmer != 0) {
        stemmerPorter sp;
        sp.stemmer(termino, tipoStemmer);
    }
    return true;
}

// ── Búsqueda ──────────────────────────────────────────────────────────────────

bool IndexadorHash::Existe(const string& word) const {
    string termino;
    if (!PrepararTermino(word, const_cast<Tokenizador&>(tok), tipoStemmer, termino)){
        return false;
    }
    return indice.find(termino) != indice.end();
}

bool IndexadorHash::Devuelve(const string& word, InformacionTermino& inf) const {
    string termino;
    if (!PrepararTermino(word, const_cast<Tokenizador&>(tok), tipoStemmer, termino)) {
        inf = InformacionTermino();
        return false;
    }

    unordered_map<string, InformacionTermino>::const_iterator it = indice.find(termino);
    if (it == indice.end()) {
        inf = InformacionTermino();
        return false;
    }

    inf = it->second;
    return true;
}

bool IndexadorHash::Devuelve(const string& word, const string& nomDoc, InfTermDoc& infDoc) const {
    unordered_map<string, class InfDoc>::const_iterator itDoc = indiceDocs.find(nomDoc);
    if (itDoc == indiceDocs.end()) {
        infDoc = InfTermDoc();
        return false;
    }
    int idDoc = itDoc->second.idDoc;

    string termino;
    if (!PrepararTermino(word, const_cast<Tokenizador&>(tok), tipoStemmer, termino)) {
        infDoc = InfTermDoc();
        return false;
    }

    unordered_map<string, InformacionTermino>::const_iterator itTerm = indice.find(termino);
    if (itTerm != indice.end()) {
        unordered_map<int, class InfTermDoc>::const_iterator itAparicion = itTerm->second.l_docs.find(idDoc);
        if (itAparicion != itTerm->second.l_docs.end()) {
            infDoc = itAparicion->second;
            return true;
        }
    }

    infDoc = InfTermDoc();
    return false;
}

// ── Pregunta ──────────────────────────────────────────────────────────────────

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
    string termino;
    if (!PrepararTermino(word, const_cast<Tokenizador&>(tok), tipoStemmer, termino)) {
        inf = InformacionTerminoPregunta();
        return false;
    }

    unordered_map<string, InformacionTerminoPregunta>::const_iterator it = indicePregunta.find(termino);
    if (it == indicePregunta.end()) {
        inf = InformacionTerminoPregunta();
        return false;
    }

    inf = it->second;
    return true;
}

bool IndexadorHash::IndexarPregunta(const string& preg) {
    VaciarIndicePreg();
    pregunta = preg;

    list<string> tokens;
    tok.Tokenizar(preg, tokens);
    if (tokens.empty()){
        return false;
    }

    stemmerPorter sp;
    int pos = 0;

    for (list<string>::iterator it = tokens.begin(); it != tokens.end(); ++it) {
        infPregunta.numTotalPal++;

        if (stopWords.find(*it) != stopWords.end()) {
            pos++;
            continue;
        }

        string& termIndexar = *it;
        if (tipoStemmer != 0) {
            sp.stemmer(termIndexar, tipoStemmer);
        }

        infPregunta.numTotalPalSinParada++;
        InformacionTerminoPregunta& itp = indicePregunta[termIndexar];

        if (itp.ft == 0) {
            infPregunta.numTotalPalDiferentes++;
        }
        itp.ft++;

        if (almacenarPosTerm){
            itp.posTerm.push_back(pos);
        }
        pos++;
    }
    return true;
}

// ── Borrar ────────────────────────────────────────────────────────────────────

bool IndexadorHash::BorraDoc(const string& nomDoc) {
    unordered_map<string, InfDoc>::iterator itDoc = indiceDocs.find(nomDoc);
    if (itDoc == indiceDocs.end()) {
        return false;
    }

    int idABorrar = itDoc->second.idDoc;

    informacionColeccionDocs.numDocs--;
    informacionColeccionDocs.numTotalPal -= itDoc->second.numPal;
    informacionColeccionDocs.numTotalPalSinParada -= itDoc->second.numPalSinParada;
    informacionColeccionDocs.tamBytes -= itDoc->second.tamBytes;

    for (unordered_map<string, InformacionTermino>::iterator itTerm = indice.begin(); itTerm != indice.end(); ) {
        unordered_map<int, InfTermDoc>::iterator itAparicion = itTerm->second.l_docs.find(idABorrar);
        if (itAparicion != itTerm->second.l_docs.end()) {
            itTerm->second.ftc -= itAparicion->second.ft;
            itTerm->second.l_docs.erase(itAparicion);

            if (itTerm->second.l_docs.empty()) {
                itTerm = indice.erase(itTerm);
                continue;
            }
        }
        ++itTerm;
    }

    indiceDocs.erase(itDoc);
    informacionColeccionDocs.numTotalPalDiferentes = indice.size();
    return true;
}

// ── Indexar ───────────────────────────────────────────────────────────────────

bool IndexadorHash::Indexar(const string& ficheroDocumentos) {
    ifstream lista(ficheroDocumentos.c_str());
    if (!lista) {
        return false;
    }

    stemmerPorter sp;
    int maxID = indiceDocs.size();

    string nomFich;
    while (getline(lista, nomFich)) {
        if (nomFich.empty()) {
            continue;
        }

        struct stat st;
        if (stat(nomFich.c_str(), &st) != 0) {
            continue;
        }

        int idAsignado = ++maxID;
        unordered_map<string, InfDoc>::iterator itExistente = indiceDocs.find(nomFich);
        if (itExistente != indiceDocs.end()) {
            if (st.st_mtime > itExistente->second.fechaModificacion) {
                idAsignado = itExistente->second.idDoc;
                BorraDoc(nomFich);
                maxID--;
            }
            else {
                continue;
            }
        }

        ifstream fDoc(nomFich.c_str());
        if (!fDoc) {
            continue;
        }

        InfDoc infoD;
        infoD.idDoc = idAsignado;
        infoD.tamBytes = st.st_size;
        infoD.fechaModificacion = st.st_mtime;

        string linea;
        list<string> tokens;  // reutilizado en cada línea
        int pos = 0;

        while (getline(fDoc, linea)) {
            if (!linea.empty() && linea[linea.length() - 1] == '\r') {
                linea.erase(linea.length() - 1);
            }
            if (linea.empty()) continue;

            tokens.clear();  // reutilizar el contenedor sin realocar
            tok.Tokenizar(linea, tokens);

            for (list<string>::iterator it = tokens.begin(); it != tokens.end(); ++it) {
                string& palabra = *it;  // referencia, sin copia
                if (palabra.empty()) continue;

                infoD.numPal++;

                if (stopWords.find(palabra) != stopWords.end()) {
                    pos++;
                    continue;
                }

                infoD.numPalSinParada++;

                // Stemmer modifica palabra in-place, sin string intermedia
                if (tipoStemmer != 0) {
                    sp.stemmer(palabra, tipoStemmer);
                }

                InformacionTermino& infT = indice[palabra];
                InfTermDoc& itd = infT.l_docs[idAsignado];

                if (itd.ft == 0) {
                    infoD.numPalDiferentes++;
                }
                itd.ft++;
                infT.ftc++;

                if (almacenarPosTerm) {
                    itd.posTerm.push_back(pos);
                }
                pos++;
            }
        }
        fDoc.close();

        indiceDocs[nomFich] = infoD;
        informacionColeccionDocs.numDocs++;
        informacionColeccionDocs.numTotalPal += infoD.numPal;
        informacionColeccionDocs.numTotalPalSinParada += infoD.numPalSinParada;
        informacionColeccionDocs.tamBytes += infoD.tamBytes;
    }

    informacionColeccionDocs.numTotalPalDiferentes = indice.size();
    return true;
}

// ── IndexarDirectorio ─────────────────────────────────────────────────────────

bool IndexadorHash::IndexarDirectorio(const string& dirAIndexar) {
    DIR* dp = opendir(dirAIndexar.c_str());
    if (!dp){
        return false;
    }

    vector<string> ficheros;
    struct dirent* dirp;
    while ((dirp = readdir(dp)) != NULL) {
        string name = string(dirp->d_name);
        if (name != "." && name != "..") {
            ficheros.push_back(dirAIndexar + "/" + name);
        }
    }
    closedir(dp);

    sort(ficheros.begin(), ficheros.end());

    string listaTmp = "lista_tmp.txt";
    ofstream ftmp(listaTmp);
    for (vector<string>::iterator it = ficheros.begin(); it != ficheros.end(); ++it){
        ftmp << *it << endl;
    }
    ftmp.close();

    bool res = Indexar(listaTmp);
    remove(listaTmp.c_str());
    return res;
}

// ── Guardar y recuperar ───────────────────────────────────────────────────────

bool IndexadorHash::GuardarIndexacion() const {
    string dir = directorioIndice;
    if (dir.empty()) {
        dir = ".";
    }
    mkdir(dir.c_str(), 0755);

    ofstream f((dir + "/indexacion.dat").c_str());
    if (!f) {
        return false;
    }

    f << ficheroStopWords << "\n";
    f << tok.DelimitadoresPalabra() << "\n";
    f << const_cast<Tokenizador&>(tok).CasosEspeciales() << "\n";
    f << const_cast<Tokenizador&>(tok).PasarAminuscSinAcentos() << "\n";
    f << directorioIndice << "\n";
    f << tipoStemmer << "\n";
    f << almacenarPosTerm << "\n";
    f << pregunta << "\n";

    f << stopWords.size() << "\n";
    for (unordered_set<string>::const_iterator it = stopWords.begin(); it != stopWords.end(); ++it){
        f << *it << "\n";
    }

    f << informacionColeccionDocs.numDocs << "\n";
    f << informacionColeccionDocs.numTotalPal << "\n";
    f << informacionColeccionDocs.numTotalPalSinParada << "\n";
    f << informacionColeccionDocs.numTotalPalDiferentes << "\n";
    f << informacionColeccionDocs.tamBytes << "\n";

    f << indiceDocs.size() << "\n";
    for (unordered_map<string, InfDoc>::const_iterator it = indiceDocs.begin(); it != indiceDocs.end(); ++it) {
        f << it->first << "\n";
        f << it->second.idDoc << "\n";
        f << it->second.numPal << "\n";
        f << it->second.numPalSinParada << "\n";
        f << it->second.numPalDiferentes << "\n";
        f << it->second.tamBytes << "\n";
        f << it->second.fechaModificacion << "\n";
    }

    f << indice.size() << "\n";
    for (unordered_map<string, InformacionTermino>::const_iterator it = indice.begin(); it != indice.end(); ++it) {
        f << it->first << "\n";
        f << it->second.ftc << "\n";
        f << it->second.l_docs.size() << "\n";
        for (unordered_map<int, InfTermDoc>::const_iterator itD = it->second.l_docs.begin(); itD != it->second.l_docs.end(); ++itD) {
            f << itD->first << "\n";
            f << itD->second.ft << "\n";
            f << itD->second.posTerm.size() << "\n";
            for (vector<int>::const_iterator itP = itD->second.posTerm.begin(); itP != itD->second.posTerm.end(); ++itP)
                f << *itP << "\n";
        }
    }

    f << indicePregunta.size() << "\n";
    for (unordered_map<string, InformacionTerminoPregunta>::const_iterator it = indicePregunta.begin(); it != indicePregunta.end(); ++it) {
        f << it->first << "\n";
        f << it->second.ft << "\n";
        f << it->second.posTerm.size() << "\n";
        for (vector<int>::const_iterator itP = it->second.posTerm.begin(); it->second.posTerm.end() != itP; ++itP)
            f << *itP << "\n";
    }

    f << infPregunta.numTotalPal << "\n";
    f << infPregunta.numTotalPalSinParada << "\n";
    f << infPregunta.numTotalPalDiferentes << "\n";

    return true;
}

bool IndexadorHash::RecuperarIndexacion(const string& directorioIndexacion) {
    VaciarIndiceDocs();
    VaciarIndicePreg();
    stopWords.clear();

    string dir = directorioIndexacion;
    if (dir.empty()) {
        dir = ".";
    }
    ifstream f((dir + "/indexacion.dat").c_str());
    if (!f) {
        return false;
    }

    getline(f, ficheroStopWords);
    string delims;
    getline(f, delims);
    bool casosEsp, minusc;
    f >> casosEsp >> minusc;
    f.ignore();
    getline(f, directorioIndice);
    f >> tipoStemmer >> almacenarPosTerm;
    f.ignore();
    getline(f, pregunta);

    tok.DelimitadoresPalabra(delims);
    tok.CasosEspeciales(casosEsp);
    tok.PasarAminuscSinAcentos(minusc);

    int nSW; f >> nSW;
    f.ignore();
    for (int i = 0; i < nSW; i++) {
        string sw;
        getline(f, sw);
        stopWords.insert(sw);
    }

    f >> informacionColeccionDocs.numDocs;
    f >> informacionColeccionDocs.numTotalPal;
    f >> informacionColeccionDocs.numTotalPalSinParada;
    f >> informacionColeccionDocs.numTotalPalDiferentes;
    f >> informacionColeccionDocs.tamBytes;
    f.ignore();

    int nDocs;
    f >> nDocs;
    f.ignore();
    for (int i = 0; i < nDocs; i++) {
        string nomDoc;
        getline(f, nomDoc);
        InfDoc d;
        f >> d.idDoc >> d.numPal >> d.numPalSinParada >> d.numPalDiferentes >> d.tamBytes >> d.fechaModificacion;
        f.ignore();
        indiceDocs[nomDoc] = d;
    }

    int nTerms;
    f >> nTerms;
    f.ignore();
    // Reservar antes de insertar para evitar rehashes durante la carga
    indice.reserve(nTerms * 2);
    for (int i = 0; i < nTerms; i++) {
        string term;
        getline(f, term);
        InformacionTermino it;
        f >> it.ftc;
        int nDocs2;
        f >> nDocs2;
        f.ignore();
        for (int j = 0; j < nDocs2; j++) {
            int idDoc;
            f >> idDoc;
            InfTermDoc itd;
            f >> itd.ft;
            int nPos;
            f >> nPos;
            f.ignore();
            itd.posTerm.reserve(nPos);  // reserva exacta, sin reallocs
            for (int k = 0; k < nPos; k++) {
                int pos;
                f >> pos;
                itd.posTerm.push_back(pos);
            }
            if (nPos > 0) {
                f.ignore();
            }
            it.l_docs[idDoc] = itd;
        }
        indice[term] = it;
    }

    int nPreg;
    f >> nPreg;
    f.ignore();
    for (int i = 0; i < nPreg; i++) {
        string term;
        getline(f, term);
        InformacionTerminoPregunta itp;
        f >> itp.ft;
        int nPos; f >> nPos;
        f.ignore();
        itp.posTerm.reserve(nPos);  // reserva exacta
        for (int k = 0; k < nPos; k++) {
            int pos;
            f >> pos;
            itp.posTerm.push_back(pos);
        }
        if (nPos > 0){
            f.ignore();
        }
        indicePregunta[term] = itp;
    }

    f >> infPregunta.numTotalPal >> infPregunta.numTotalPalSinParada >> infPregunta.numTotalPalDiferentes;

    return true;
}