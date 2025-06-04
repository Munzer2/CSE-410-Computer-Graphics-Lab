#include <bits/stdc++.h>
#include "Matrix.h"
#include "vector.h"
#include "Parser.h"
using namespace std;



int main() { 
    Parser check("Test Cases/4/scene.txt", "Outputs/stage1.txt");
    // check.PrintCamAndPers();
    check._Parse(); 
    return 0;
}