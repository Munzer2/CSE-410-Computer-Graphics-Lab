#include <bits/stdc++.h>
#include "Matrix.h"
#include "vector.h"
#include "Parser.h"
using namespace std;



int main() { 
    Parser check("Test Cases/1/scene.txt", "Outputs", "Test Cases/1/config.txt");
    // check.PrintCamAndPers();
    check._Parse(); 
    return 0;
}