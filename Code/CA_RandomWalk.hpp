#include<iostream>
#include<cmath>
#include<string>
#include<fstream>

#include"Random64.hpp"

const int Lx = 200;
const int Ly = 200;
const int N = 400;

Crandom ran64(1);
const double p = 0.5;

class RWLattice{
    private:
        int *coffee = NULL;
    public:
        RWLattice(unsigned long long seed);
        ~RWLattice();
        int get1D(int ix, int iy);
        void initialize(void);
        void propagate(void);
        bool check(void);
        void save(std::string filename);
};

RWLattice::RWLattice(unsigned long long seed){
    coffee = new int[Lx*Ly];
}

RWLattice::~RWLattice(){
    delete[] coffee;
}

int RWLattice::get1D(int ix, int iy){
    return ix*Ly + iy;
}

void RWLattice::initialize(void){
    // All zeros
    #pragma opm parallel for
    for(int i=0; i<Lx*Ly; i++)
        coffee[i] = 0;
    // Place molecules
    for(int ix=90; ix<110; ix++)
        for(int iy=90; iy<110; iy++){
            int pos = get1D(ix, iy);
            coffee[pos] = 1;
        }
    if(check() == false) throw "Bad initialization";
}

void RWLattice::propagate(void){
    for (int ix=0; ix<Lx; ix++)
        for (int iy=0; iy<Ly; iy++){
            int pos = get1D(ix, iy);
            if(coffee[pos] == 0) continue;

            double r_1 = ran64.r(); double r_2 = ran64.r();
            if(p > r_1){ // Moves in x
                if(p > r_2 && ix != Lx-1) coffee[get1D(ix+1, iy)]++; // Right
                else if(ix != 0) coffee[get1D(ix-1, iy)]++; // Left
                else if(ix == 0 || ix == Lx-1) continue;
            }
            else{ // Moves in y
                if(p > r_2 && iy != Ly-1) coffee[get1D(ix, iy+1)]++; // Up
                else if(iy != 0) coffee[get1D(ix, iy)]++; // Down
                else if(iy == 0 || iy == Ly-1) continue;
            }
            coffee[pos]--;

        }
}

bool RWLattice::check(void){
    int sum = 0;
    #pragma opm parallel for reduction(+:sum)
    for(int i=0; i<Lx*Ly; i++)
        sum += coffee[i];
    
    if(sum == N) return true;
    else return false;
}

void RWLattice::save(std::string filename){
    std::ofstream File(filename);
    for(int ix=0; ix<Lx; ix++){
        for(int iy=0; iy<Ly; iy++){
            int pos = get1D(ix, iy);
            File << ix << '\t' << iy << '\t' << coffee[pos]  << '\n';
            }
        File << '\n';
    }
    File << std::endl;
    File.close();
}