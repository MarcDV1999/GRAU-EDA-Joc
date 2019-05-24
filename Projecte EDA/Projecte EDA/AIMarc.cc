#include "Player.hh"

/**
 * Write the name of your player and save this file
 * with the same name and .cc extension.
 */
#define PLAYER_NAME Marc


struct PLAYER_NAME : public Player {
    
    
    /**
     * Factory: returns a new instance of this class.
     * Do not modify this function.
     */
    static Player* factory () {
        return new PLAYER_NAME;
    }
    
    /**
     * Types and attributes for your player can be defined here.
     */
    typedef vector<int> vec_int;
    struct pos_bloqueo{
        Pos p;
        int contador;
        pos_bloqueo(Pos p1, int contador1) :  p(p1), contador(contador1){}
    };
    
    
    const int pos_rel[8] = {0,1,0,-1,1,0,-1,0};
    const Dir direcciones[4] = {Down,Up,Left,Right};
    set<Pos> menjar_reina;
    set<Pos> pos_ocupades;
    int contador_nutrientes = 0;
    
    
    /**
     * Play method, invoked once per each round.
     */
    virtual void play () {
        // Vectors amb els ID de totes les Formigues
        vec_int id_workers = workers(me());
        vec_int id_soldiers = soldiers(me());
        vec_int id_queens = queens(me());
        
        //cerr << "empiezo" << endl;
        
        init_pos_ocupades( id_workers, id_soldiers,id_queens);
        /*
         for (auto i : id_workers)cerr << i <<",";
         cerr << " --> Workers"<< endl;
         for (auto i : id_soldiers)cerr << i <<",";
         cerr << " --> Soldiers"<< endl;
         for (auto i : id_queens) cerr << i <<",";
         cerr << " --> Queens"<< endl;
         */
        
        for (int i = 0; i < id_workers.size(); ++i){
            if(id_workers.size()>0){
                Ant worker = ant((id_workers[i]));
                if(worker.bonus != None) {
                    portar_menjar_reina(worker, id_queens[0]);
                }
                else anar_a_per_menjar(worker);
            }
        }
        for (int i = 0; i < id_soldiers.size(); ++i) {
            if(id_soldiers.size()>0){
                Ant soldier = ant(id_soldiers[i]);
                //move_ants_randomly(id_soldiers[i]);
                atacar_enemics(soldier);
            }
        }
        for (int i = 0; i < id_queens.size(); ++i) {
            Ant queen = ant((id_queens[i]));
            
            if(queen.reserve[0] >= soldier_carbo() and queen.reserve[1] >= soldier_prote() and queen.reserve[2] >= soldier_lipid()) {
                cerr << "Voy a poner un huevo de soldado" << endl;
                poner_huevo(queen, Soldier);
            }
            else if(id_workers.size() <= 5 and queen.reserve[0] >= worker_carbo() and queen.reserve[1] >= worker_prote() and queen.reserve[2] >= worker_lipid()) {
            //else if(queen.reserve[0] >= worker_carbo() and queen.reserve[1] >= worker_prote() and queen.reserve[2] >= worker_lipid()) {
                cerr << "Voy a poner un huevo de soldado" << endl;
                poner_huevo(queen, Worker);
            }
            else if(round()%queen_period() == 0) {
                cerr << "Voy a moverme porque no puedo hacer huevos" << endl;
                reina_va_a_menjar(queen);
            }
        }
        
        
    }
    
    /////// ---------------------------------------------------------    WORKERS     ---------------------------------------------------------
    
    queue<Dir> trobar_menjar(const Pos& start, Pos& pan){
        bool found = false;
        vector<vector<bool> > visited(board_rows(),vector<bool>(board_cols(),false));
        vector<Pos> camino;
        queue<vector<Pos> > cola_rutas; // Tots els possibles recorreguts, el bo es l'ultim
        
        camino.push_back(start);
        cola_rutas.push(camino);
        visited[start.i][start.j] = true;
        //cerr << "Vaig a entrar al while" << endl;
        while(not cola_rutas.empty() and not found){
            //cerr << "Entro al while" << endl;
            camino = cola_rutas.front(); cola_rutas.pop();
            Pos actual = camino.back();
            if(cell(actual).bonus != None and menjar_reina.find(actual) == menjar_reina.end()) {
                found = true;
                pan = camino.back();
                //cerr << "\t^^^^^He trobat Menjar a la pos: " << camino.back() << endl;
            }
            else{
                
                //cerr << "No He trobat res" << endl;
                for (Dir d_siguiente : direcciones) {
                    //cerr << "\tInvestigo a la direccio " << d_siguiente << endl;
                    Pos siguiente = actual + d_siguiente;
                    if(pos_ok(siguiente) and cell(siguiente).type != Water and not visited[siguiente.i][siguiente.j] and cell(siguiente).id == -1){
                        //cerr << "\t^^^^^Cumpleix les condicions aixi que l'afegeixo a la cua: " << camino.back() << endl;
                        vector<Pos> new_camino = camino;
                        new_camino.push_back(siguiente);
                        cola_rutas.push(new_camino);
                        visited[siguiente.i][siguiente.j] = true;
                    }
                }
            }
            
        }
        queue<Dir> P;
        Pos actual = start;
        cerr << "__________RUTA desde " << start << " fins " << pan << " found(" << found << "): ";
        for (int i = 1; i < camino.size(); ++i) {
            Pos next = camino[i];
             cerr << next << ",";
            if(actual + Down == next)P.push(Down);
            else if(actual + Up == next)P.push(Up);
            else if(actual + Left == next)P.push(Left);
            else if(actual + Right == next)P.push(Right);
            actual = next;
        }
        //cerr << "Acabo Correctamet el bfs" << endl;
        return P;
    }
    
    void anar_a_per_menjar( const Ant& worker){
        cerr << endl << endl << "\t\tSoy la HORMIGA " << worker.pos << " y voy a por comida\t ";
        int id = worker.id;
        Pos p = worker.pos;
        Pos pan;
        Dir d;
        queue<Dir> v = trobar_menjar(p, pan);
        //cerr << "Estic a la posicio " << p << " i el menjar esta a " << pan;
        //cerr << "He Trobat menjar, aniré a buscarlo a la posicio " << pan << ": ";
        //cerr << "···\t\tLa Pos al Pan mas cercano es de: " << pan << endl;
        //cerr << "v.size: " << v.size() << endl;
        if(p == pan) {
            take(id);
            //cerr << "Acabo de agafar menjar: " << worker.bonus<< endl;
        }
        else if(v.size() == 0);
        else{
            d = v.front(); v.pop();
            //cerr << "d:" << d << endl;
            Pos siguiente_p = p + d;
            Dir d_contraria;
            if(pos_ok(siguiente_p) and cell(siguiente_p).type != Water) {
                if(pos_ocupades.find(siguiente_p) == pos_ocupades.end() and cell(siguiente_p).id == -1){
                    if(not enemigos_cerca(siguiente_p, d_contraria, d)){ // Si la posicio NO esta ocupada actualment ni en la seguent ronda
                        move(id, d);
                        pos_ocupades.insert(siguiente_p); // afegeixo on anire a les posicions ocupades
                        pos_ocupades.erase(p); // borro de les posicions ocupades la posicio antiga
                        //cerr << "___\tHormiga _" << id << "_ va a la pos (" << d << ") pos ocupada=\t" << siguiente_p << " pos libre=\t" << p<< endl;
                    }
                    else if(pos_ocupades.find(siguiente_p+d) == pos_ocupades.end() and cell(siguiente_p+d).id == -1){
                        move(id, d_contraria);
                        pos_ocupades.insert(siguiente_p); // afegeixo on anire a les posicions ocupades
                        pos_ocupades.erase(p); // borro de les posicions ocupades la posicio antiga
                        //cerr << "___\tHormiga _" << id << "_ va a la pos (" << d << ") pos ocupada=\t" << siguiente_p << " pos libre=\t" << p<< endl;
                    }
                }
            }
            //cerr << endl << endl << "\t\t\tHe aacabadooooooo " << id << endl;
        }
    }
    
    
    
    
    queue<Dir> trobar_reina(const Pos& start, Pos& queen_pos){
        bool found = false;
        
        vector<vector<bool> > visited(board_rows(),vector<bool>(board_cols(),false));
        vector<Pos> camino;
        queue<vector<Pos> > cola_rutas; // Tots els possibles recorreguts, el bo es l'ultim
        
        camino.push_back(start);
        cola_rutas.push(camino);
        visited[start.i][start.j] = true;
        //cerr << "Vaig a entrar al while" << endl;
        while(not cola_rutas.empty() and not found){
            //cerr << "Entro al while" << endl;
            camino = cola_rutas.front(); cola_rutas.pop();
            Pos actual = camino.back();
            if(ant(cell(actual).id).type == Queen and ant(cell(actual).id).player == me()) {
                found = true;
                queen_pos = camino.back();
            }
            else{
                //cerr << "No He trobat res" << endl;
                for (Dir d_siguiente : direcciones) {
                    //cerr << "\tInvestigo a la direccio " << d_siguiente << endl;
                    Pos siguiente = actual + d_siguiente;
                    if(pos_ok(siguiente) and cell(siguiente).type != Water and not visited[siguiente.i][siguiente.j]){
                        //cerr << "\t^^^^^Cumpleix les condicions aixi que l'afegeixo a la cua: " << camino.back() << endl;
                        vector<Pos> new_camino = camino;
                        new_camino.push_back(siguiente);
                        cola_rutas.push(new_camino);
                        visited[siguiente.i][siguiente.j] = true;
                    }
                }
            }
            
        }
        queue<Dir> P;
        Pos actual = start;
        //cerr << "RUTA desde " << start << " fins " << queen_pos << " found(" << found << "): ";
        for (int i = 1; i < camino.size(); ++i) {
            Pos next = camino[i];
            cerr << next << ",";
            if(actual + Down == next)P.push(Down);
            else if(actual + Up == next)P.push(Up);
            else if(actual + Left == next)P.push(Left);
            else if(actual + Right == next)P.push(Right);
            actual = next;
        }
        return P;
    }
    
    void portar_menjar_reina(const Ant& worker, const int& id_queen){
        cerr << endl << endl << "\t\t\tSoy la HORMIGA " << worker.pos  << " i voy a dar de comer a la REINA"<< endl;
        int id = worker.id;
        Pos p = worker.pos;
        Pos queen_pos;
        Dir d;
        queue<Dir> v = trobar_reina(p, queen_pos);
        //cerr << "He Trobat menjar, aniré a buscarlo a la posicio " << pan << ": ";
        //cerr << "···\t\tLa Pos al Pan mas cercano es de: " << pan << endl;
        if(v.size() <= 2){
            leave(id);
            menjar_reina.insert(p);
            //cerr << "Acabo de dejar la comida, y la comida es de la reina" << endl;
        }
        else{
            d = v.front(); v.pop();
            Pos siguiente_p = p + d;
            Dir d_contraria;
            //cerr << "La reina esta a " << queen_pos << " i jo estic a " << p << endl;
            if(pos_ok(siguiente_p) and cell(siguiente_p).type != Water) {
                if(pos_ocupades.find(siguiente_p) == pos_ocupades.end() and cell(siguiente_p).id == -1){
                    if(not enemigos_cerca(siguiente_p, d_contraria, d)){ // Si la posicio NO esta ocupada actualment ni en la seguent ronda
                        move(id, d);
                        pos_ocupades.insert(siguiente_p); // afegeixo on anire a les posicions ocupades
                        pos_ocupades.erase(p); // borro de les posicions ocupades la posicio antiga
                        //cerr << "___\tHormiga _" << id << "_ va a la pos (" << d << ") pos ocupada=\t" << siguiente_p << " pos libre=\t" << p<< endl;
                    }
                    else if(pos_ocupades.find(siguiente_p+d) == pos_ocupades.end() and cell(siguiente_p+d).id == -1){
                        move(id, d_contraria);
                        pos_ocupades.insert(siguiente_p); // afegeixo on anire a les posicions ocupades
                        pos_ocupades.erase(p); // borro de les posicions ocupades la posicio antiga
                        //cerr << "___\tHormiga _" << id << "_ va a la pos (" << d << ") pos ocupada=\t" << siguiente_p << " pos libre=\t" << p<< endl;
                    }
                }
                
            }
        }
    }
    
    bool enemigos_cerca(const Pos& siguiente_p, Dir& d_contraria, const Dir& d_actual){
        //Dir escapeDown[4] = {Up};escapeDown[4] = {}; escapeDown[4] = {}; escapeDown[4] = {};
        
        for(auto d : direcciones){
            Pos adj = siguiente_p + d;
            if(cell(adj).id != -1 and ant(cell(adj).id).player != me()) {cerr << "ESta 5" << endl;
                if(d_actual == Down and pos_ok(adj) and pos_ocupades.find(adj) == pos_ocupades.end()) d_contraria = Up;
                else if(d_actual == Up and pos_ok(adj) and pos_ocupades.find(adj) == pos_ocupades.end()) d_contraria = Down;
                else if(d_actual == Right and pos_ok(adj) and pos_ocupades.find(adj) == pos_ocupades.end()) d_contraria = Left;
                else if(d_actual == Left and pos_ok(adj) and pos_ocupades.find(adj) == pos_ocupades.end()) d_contraria = Right;
                return true;
            }
        }
        return false;
    }
    
    
    
    /////// ---------------------------------------------------------    SOLDIERS     ---------------------------------------------------------
    queue<Dir> trobar_enemics(const Pos& start, Pos& enemic_pos){
        bool found = false;
        
        vector<vector<bool> > visited(board_rows(),vector<bool>(board_cols(),false));
        vector<Pos> camino;
        queue<vector<Pos> > cola_rutas; // Tots els possibles recorreguts, el bo es l'ultim
        
        camino.push_back(start);
        cola_rutas.push(camino);
        visited[start.i][start.j] = true;
        //cerr << "Vaig a entrar al while" << endl;
        while(not cola_rutas.empty() and not found){
            //cerr << "Entro al while" << endl;
            camino = cola_rutas.front(); cola_rutas.pop();
            Pos actual = camino.back();
            if(cell(actual).id != -1 and ant(cell(actual).id).player != me() and ant(cell(actual).id).type != Queen) {
                found = true;
                enemic_pos = camino.back();
                //cerr << "\t^^^^^He trobat Menjar a la pos: " << camino.back() << endl;
            }
            else{
                //cerr << "No He trobat res" << endl;
                for (Dir d_siguiente : direcciones) {
                    //cerr << "\tInvestigo a la direccio " << d_siguiente << endl;
                    Pos siguiente = actual + d_siguiente;
                    if(pos_ok(siguiente) and cell(siguiente).type != Water and not visited[siguiente.i][siguiente.j]){
                        //cerr << "\t^^^^^Cumpleix les condicions aixi que l'afegeixo a la cua: " << camino.back() << endl;
                        vector<Pos> new_camino = camino;
                        new_camino.push_back(siguiente);
                        cola_rutas.push(new_camino);
                        visited[siguiente.i][siguiente.j] = true;
                    }
                }
            }
            
        }
        queue<Dir> P;
        Pos actual = start;
        cerr << "_RUTA desde " << start << " fins " << enemic_pos << " found(" << found << "): " << endl << "\t";
        for (int i = 1; i < camino.size(); ++i) {
            Pos next = camino[i];
            cerr << next << ",";
            if(actual + Down == next)P.push(Down);
            else if(actual + Up == next)P.push(Up);
            else if(actual + Left == next)P.push(Left);
            else if(actual + Right == next)P.push(Right);
            actual = next;
        }
        return P;
    }
    void atacar_enemics(const Ant& soldier){
        //cerr << endl << endl << "\tSoy el Soldado " << id;
        int id= soldier.id;
        Pos p = soldier.pos;
        Pos enemy_pos;
        Dir d;
        queue<Dir> v = trobar_enemics(p, enemy_pos);
        //cerr << "He Trobat menjar, aniré a buscarlo a la posicio " << pan << ": ";
        //cerr << "···\t\tLa Pos al Pan mas cercano es de: " << pan << endl;
        d = v.front(); v.pop();
        Pos siguiente_p = p + d;
        if(pos_ok(siguiente_p) and cell(siguiente_p).type != Water) {
            if(pos_ocupades.find(siguiente_p) == pos_ocupades.end() and cell(siguiente_p).id == -1){ // Si la posicio NO esta ocupada actualment ni en la seguent ronda
                move(id, d);
                pos_ocupades.insert(siguiente_p); // afegeixo on anire a les posicions ocupades
                pos_ocupades.erase(p); // borro de les posicions ocupades la posicio antiga
                //cerr << "___\tHormiga _" << id << "_ va a la pos (" << d << ") pos ocupada=\t" << siguiente_p << " pos libre=\t" << p<< endl;
            }
        }
    }
    
    /////// ---------------------------------------------------------    REINA     ---------------------------------------------------------
    
    void poner_huevo(const Ant& queen, AntType a){
        int id = queen.id;
        for (Dir d : direcciones) {
            //cerr << "quiero poner un huevo a " << d << endl;
            Pos adj = queen.pos + d;
            //cerr << "tipo de pos: " << cell(adj).type << endl;
            if(pos_ok(adj) and cell(adj).type != Water and pos_ocupades.find(adj) == pos_ocupades.end()) {
                //cerr << "tipo de pos: " << cell(adj).type << endl;
                //cerr << "Pos ok: " << pos_ok(adj) << endl;
                cerr << "La Reina acaba de Poner un huevo de " << a  << "en la pos " << d << endl;
                lay(id, d, a);
                return;
            }
            
        }
    }
    
    
    void reina_va_a_menjar( const Ant& queen){
        cerr << endl << endl << "\t\t\tSoy la REINA " << queen.pos << endl;
        int id = queen.id;
        Pos p = queen.pos;
        Pos menjar;
        Dir d;
        queue<Dir> v = trobar_menjar_reina(p, menjar);
        //cerr << "__Estic a la posicio " << p << " i el menjar esta a " << menjar << endl;
        //cerr << "He Trobat menjar, aniré a buscarlo a la posicio " << pan << ": ";
        //cerr << "···\t\tLa Pos al Pan mas cercano es de: " << pan << endl;
        //cerr << "v.size: " << v.size() << endl;
        if(v.size() == 0);
        else{
            d = v.front(); v.pop();
            //cerr << "d:" << d << endl;
            Pos siguiente_p = p + d;
            if(pos_ok(siguiente_p) and cell(siguiente_p).type != Water) {
                if(pos_ocupades.find(siguiente_p) == pos_ocupades.end() and cell(siguiente_p).id == -1){ // Si la posicio NO esta ocupada actualment ni en la seguent ronda
                    move(id, d);
                    
                    pos_ocupades.insert(siguiente_p); // afegeixo on anire a les posicions ocupades
                    pos_ocupades.erase(p); // borro de les posicions ocupades la posicio antiga
                    //cerr << "___\tHormiga _" << id << "_ va a la pos (" << d << ") pos ocupada=\t" << siguiente_p << " pos libre=\t" << p<< endl;
                }
            }
            //cerr << endl << endl << "\t\t\tHe aacabadooooooo " << id << endl;
        }
        
    }
    
    queue<Dir> trobar_menjar_reina(const Pos& start, Pos& menjar){
        bool found = false;
        //cerr << "Menjar" << endl;
        vector<vector<bool> > visited(board_rows(),vector<bool>(board_cols(),false));
        vector<Pos> camino;
        queue<vector<Pos> > cola_rutas; // Tots els possibles recorreguts, el bo es l'ultim
        
        camino.push_back(start);
        cola_rutas.push(camino);
        visited[start.i][start.j] = true;
        //cerr << "Vaig a entrar al while" << endl;
        while(not cola_rutas.empty() and not found){
            //cerr << "Entro al while" << endl;
            camino = cola_rutas.front(); cola_rutas.pop();
            Pos actual = camino.back();
            if(cell(actual).bonus != None) {
                cerr << "Voy a por comida, Posicion: " << actual << endl;
                found = true;
                menjar = camino.back();
                //cerr << "\t^^^^^He trobat Menjar a la pos: " << camino.back() << endl;
            }
            else{
                
                //cerr << "No He trobat res" << endl;
                for (Dir d_siguiente : direcciones) {
                    //cerr << "\tInvestigo a la direccio " << d_siguiente << endl;
                    Pos siguiente = actual + d_siguiente;
                    if(pos_ok(siguiente) and cell(siguiente).type != Water and not visited[siguiente.i][siguiente.j] and cell(siguiente).id == -1){
                        //cerr << "\t^^^^^Cumpleix les condicions aixi que l'afegeixo a la cua: " << camino.back() << endl;
                        vector<Pos> new_camino = camino;
                        new_camino.push_back(siguiente);
                        cola_rutas.push(new_camino);
                        visited[siguiente.i][siguiente.j] = true;
                    }
                }
            }
            
        }
        queue<Dir> P;
        Pos actual = start;
        //cerr << "__________RUTA desde " << start << " fins " << pan << " found(" << found << "): ";
        for (int i = 1; i < camino.size(); ++i) {
            Pos next = camino[i];
            // cerr << next << ",";
            if(actual + Down == next)P.push(Down);
            else if(actual + Up == next)P.push(Up);
            else if(actual + Left == next)P.push(Left);
            else if(actual + Right == next)P.push(Right);
            actual = next;
        }
        //cerr << "Acabo Correctamet el bfs" << endl;
        return P;
    }
    
    
    
    
    void move_ants_randomly( const int& id){
        //cerr << endl << endl << "\t\t\tSoy la HORMIGA " << id << endl;
        int i = random(0, 4), j = 0;
        bool moved = false;
        Ant worker = ant(id);
        Pos p = worker.pos;
        Pos pa;
        while(!moved and j < 4){ // Si encara no m'he mogut o ja han passat 4 intents de moure's
            Dir d = direcciones[i];
            Pos siguiente_p = p + d;
            if(pos_ok(siguiente_p) and cell(siguiente_p).type != Water) {
                if(pos_ocupades.find(siguiente_p) == pos_ocupades.end() and cell(siguiente_p).id == -1){ // Si la posicio NO esta ocupada actualment ni en la seguent ronda
                    move(id, d);
                    pos_ocupades.insert(siguiente_p); // afegeixo on anire a les posicions ocupades
                    pos_ocupades.erase(p); // borro de les posicions ocupades la posicio antiga
                    moved = true;
                    //cerr << "___\tHormiga _" << id << "_ va a la pos (" << d << ") pos ocupada=\t" << siguiente_p << " pos libre=\t" << p<< endl;
                }
                else {
                    ++j;
                    i = random(0, 4);
                    //cerr << "···\tHormiga ·" << id << "· no puedo ir a la pos (" << d << ")"<< siguiente_p << " porque esta la hormiga " << cell(siguiente_p).id<< endl;
                }
            }
            else {
                ++j;
                i = random(0, 4);
                //cerr << "Hormiga " << id << " no puedo ir a la pos (" << d << ")"<< siguiente_p << " porque esta la hormiga " << cell(siguiente_p).id<< endl;
            }
        }
    }
    
    
    
    
    
    
    void init_pos_ocupades(vec_int id_workers, vec_int id_soldiers, vec_int id_queens){
        //cerr << "Tinc " << pos_ocupades.size() << " posicions Ocupades" << endl;
        pos_ocupades.clear();
        cerr << "Menjar reina: " << menjar_reina.size() << endl;
        //cerr << "Tinc " << pos_ocupades.size() << " posicions Ocupades" << endl;
        for(int i = 0; i < id_workers.size();++i) {
            Ant worker = ant(id_workers[i]);
            pos_ocupades.insert(worker.pos);
        }
        for(int i = 0; i < id_soldiers.size();++i) {
            Ant soldier = ant(id_soldiers[i]);
            pos_ocupades.insert(soldier.pos);
        }
        for(int i = 0; i < id_queens.size();++i) {
            Ant queen = ant(id_queens[i]);
            pos_ocupades.insert(queen.pos);
            
        }
        /*
         for(auto i : pos_ocupades){
         cerr << i << ",";
         }
         cerr << endl;
         */
    }
    
    bool winning() {
        for (int pl = 0; pl < num_players(); ++pl)
            if (pl != me() and score(me()) <= score(pl))
                return false;
        return true;
    }
    
    
};


/**
 * Do not modify the following line.
 */
RegisterPlayer(PLAYER_NAME);
