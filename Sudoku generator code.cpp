//This code is Developed by 22CS008,22CS014,22CS033.
// This will generate a svg file format or sudoku puzzle. Which you can fetch easily.
// For example you can use it as a online Sudoku website generator.

#include <iostream>
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#define UNASSIGNED 0

using namespace std;
//! Step 1 : Initialize the object first
//! Set up the initial position of the puzzle
class Sudoku {
private:
  int grid[9][9];
  int solnGrid[9][9];
  int guessNum[9];
  int gridPos[81];
  int difficultyLevel;
  bool grid_status;

public:
  Sudoku ();
  Sudoku (string, bool row_major=true);
  void fillEmptyDiagonalBox(int);
  void createSeed();
  void printGrid();
  bool solveGrid();
  string getGrid();
  void countSoln(int &number);
  void genPuzzle();
  bool verifyGridStatus();
  void printSVG(string);
  void calculateDifficulty();
  int  branchDifficultyScore();
};

// START: Get grid as string in row major order
string Sudoku::getGrid()
{
  string s = "";
  for(int row_num=0; row_num<9; ++row_num)
  {
    for(int col_num=0; col_num<9; ++col_num)
    {
      s = s + to_string(grid[row_num][col_num]);
    }
  }

  return s;
}
// END: Get grid as string in row major order


// START: Generate random number
int genRandNum(int maxLimit)
{
  return rand()%maxLimit;
}
// END: Generate random number


// START: Helper functions for solving grid
//! Step 4 : Implement the helper function to solve the Sudoku
bool FindUnassignedLocation(int grid[9][9], int &row, int &col)
{
    for (row = 0; row < 9; row++)
    {
        for (col = 0; col < 9; col++)
        {
            if (grid[row][col] == UNASSIGNED)
                return true;
        }
    }

    return false;
}

//! Step 4 : Use Helper function to check if numbr is present in row or not
bool UsedInRow(int grid[9][9], int row, int num)
{
    for (int col = 0; col < 9; col++)
    {
        if (grid[row][col] == num)
            return true;
    }

    return false;
}
//! Step 5 : Use helper function to check if number is present in column or not
bool UsedInCol(int grid[9][9], int col, int num)
{
    for (int row = 0; row < 9; row++)
    {
        if (grid[row][col] == num)
            return true;
    }

    return false;
}
//! Step 6 : Use Helper function to check if number is present in a 3 x 3 matrix or not
bool UsedInBox(int grid[9][9], int boxStartRow, int boxStartCol, int num)
{
    for (int row = 0; row < 3; row++)
    {
        for (int col = 0; col < 3; col++)
        {
            if (grid[row+boxStartRow][col+boxStartCol] == num)
                return true;
        }
    }

    return false;
}
//! Step 7 : Use Helper Function to check if number is Used in Row, Column and Box
bool isSafe(int grid[9][9], int row, int col, int num)
{
    return !UsedInRow(grid, row, num) && !UsedInCol(grid, col, num) && !UsedInBox(grid, row - row%3 , col - col%3, num);
}
// END: Helper functions for solving grid


// START: Create seed grid
//! Step 9 : Create a seed grid to generate a random number
void Sudoku::fillEmptyDiagonalBox(int idx)
{
  int start = idx*3;
  random_shuffle(this->guessNum, (this->guessNum) + 9, genRandNum);
  for (int i = 0; i < 3; ++i)
  {
    for (int j = 0; j < 3; ++j)
    {
      this->grid[start+i][start+j] = guessNum[i*3+j];
    }
  }
}

//! Step 10 : Create a helper funcion for seed grid
void Sudoku::createSeed()
{
  /* Fill diagonal boxes to form:
      x | . | .
      . | x | .
      . | . | x
  */
  this->fillEmptyDiagonalBox(0);
  this->fillEmptyDiagonalBox(1);
  this->fillEmptyDiagonalBox(2);

  /* Fill the remaining blocks:
      x | x | x
      x | x | x
      x | x | x
  */
  this->solveGrid(); // TODO: not truly random, but still good enough because we generate random diagonals.

  // Saving the solution grid
  for(int i=0;i<9;i++)
  {
    for(int j=0;j<9;j++)
    {
      this->solnGrid[i][j] = this->grid[i][j];
    }
  }
}
// END: Create seed grid


// START: Intialising
//!Step 2 : Defualt Constructor
Sudoku::Sudoku()
{

  // initialize difficulty level
  this->difficultyLevel = 0;

  // Randomly shuffling the array of removing grid positions
  for(int i=0;i<81;i++)
  {
    this->gridPos[i] = i;
  }

  random_shuffle(this->gridPos, (this->gridPos) + 81, genRandNum);

  // Randomly shuffling the guessing number array
  for(int i=0;i<9;i++)
  {
    this->guessNum[i]=i+1;
  }

  random_shuffle(this->guessNum, (this->guessNum) + 9, genRandNum);

  // Initialising the grid
  for(int i=0;i<9;i++)
  {
    for(int j=0;j<9;j++)
    {
      this->grid[i][j]=0;
    }
  }

  grid_status = true;
}
// END: Initialising

//! Step 11 : Create a Parameterized Constructor
// START: Custom Initialising with grid passed as argument
Sudoku::Sudoku(string grid_str, bool row_major)
{
  if(grid_str.length() != 81)
  {
    grid_status=false;
    return;
  }

  // First pass: Check if all cells are valid
  for(int i=0; i<81; ++i)
  {
    int curr_num = grid_str[i]-'0';
    if(!((curr_num == UNASSIGNED) || (curr_num > 0 && curr_num < 10)))
    {
      grid_status=false;
      return;
    }

    if(row_major) grid[i/9][i%9] = curr_num;
    else          grid[i%9][i/9] = curr_num;
  }

  // Second pass: Check if all columns are valid
  for (int col_num=0; col_num<9; ++col_num)
  {
    bool nums[10]={false};
    for (int row_num=0; row_num<9; ++row_num)
    {
      int curr_num = grid[row_num][col_num];
      if(curr_num!=UNASSIGNED && nums[curr_num]==true)
      {
        grid_status=false;
        return;
      }
      nums[curr_num] = true;
    }
  }

  // Third pass: Check if all rows are valid
  for (int row_num=0; row_num<9; ++row_num)
  {
    bool nums[10]={false};
    for (int col_num=0; col_num<9; ++col_num)
    {
      int curr_num = grid[row_num][col_num];
      if(curr_num!=UNASSIGNED && nums[curr_num]==true)
      {
        grid_status=false;
        return;
      }
      nums[curr_num] = true;
    }
  }

  // Fourth pass: Check if all blocks are valid
  for (int block_num=0; block_num<9; ++block_num)
  {
    bool nums[10]={false};
    for (int cell_num=0; cell_num<9; ++cell_num)
    {
      int curr_num = grid[((int)(block_num/3))*3 + (cell_num/3)][((int)(block_num%3))*3 + (cell_num%3)];
      if(curr_num!=UNASSIGNED && nums[curr_num]==true)
      {
        grid_status=false;
        return;
      }
      nums[curr_num] = true;
    }
  }

  // Randomly shuffling the guessing number array
  for(int i=0;i<9;i++)
  {
    this->guessNum[i]=i+1;
  }

  random_shuffle(this->guessNum, (this->guessNum) + 9, genRandNum);

  grid_status = true;
}
// END: Custom Initialising


// START: Verification status of the custom grid passed
//! Check if Grid is valid or not
bool Sudoku::verifyGridStatus()
{
  return grid_status;
}
// END: Verification of the custom grid passed


// START: Printing the grid
void Sudoku::printGrid()
{
  for(int i=0;i<9;i++)
  {
    for(int j=0;j<9;j++)
    {
      if(grid[i][j] == 0)
	cout<<".";
      else
	cout<<grid[i][j];
      cout<<"|";
    }
    cout<<endl;
  }

  cout<<"\nDifficulty of current sudoku(0 being easiest): "<<this->difficultyLevel;
  cout << "\nCreated By 22CS008, 22CS014, 22CS033";
  cout << endl;
}
// END: Printing the grid


// START: Modified Sudoku solver
bool Sudoku::solveGrid()
{
    int row, col;

    // If there is no unassigned location, we are done
    if (!FindUnassignedLocation(this->grid, row, col))
       return true; // success!

    // Consider digits 1 to 9
    for (int num = 0; num < 9; num++)
    {
        // if looks promising
        if (isSafe(this->grid, row, col, this->guessNum[num]))
        {
            // make tentative assignment
            this->grid[row][col] = this->guessNum[num];

            // return, if success, yay!
            if (solveGrid())
                return true;

            // failure, unmake & try again
            this->grid[row][col] = UNASSIGNED;
        }
    }

    return false; // this triggers backtracking

}
// END: Modified Sudoku Solver


// START: Check if the grid is uniquely solvable
void Sudoku::countSoln(int &number)
{
  int row, col;

  if(!FindUnassignedLocation(this->grid, row, col))
  {
    number++;
    return ;
  }


  for(int i=0;i<9 && number<2;i++)
  {
      if( isSafe(this->grid, row, col, this->guessNum[i]) )
      {
        this->grid[row][col] = this->guessNum[i];
        countSoln(number);
      }

      this->grid[row][col] = UNASSIGNED;
  }

}
// END: Check if the grid is uniquely solvable


// START: Gneerate puzzle
void Sudoku::genPuzzle()
{
  for(int i=0;i<81;i++)
  {
    int x = (this->gridPos[i])/9;
    int y = (this->gridPos[i])%9;
    int temp = this->grid[x][y];
    this->grid[x][y] = UNASSIGNED;

    // If now more than 1 solution , replace the removed cell back.
    int check=0;
    countSoln(check);
    if(check!=1)
    {
      this->grid[x][y] = temp;
    }
  }
}
// END: Generate puzzle


// START: Printing into SVG file
void Sudoku::printSVG(string path="")
{
  string fileName = path + "svgHead.txt";
  ifstream file1(fileName.c_str());
  stringstream svgHead;
  svgHead << file1.rdbuf();

  ofstream outFile("puzzle.svg");
  outFile << svgHead.rdbuf();

  for(int i=0;i<9;i++)
  {
    for(int j=0;j<9;j++)
    {
      if(this->grid[i][j]!=0)
      {
        int x = 50*j + 16;
        int y = 50*i + 35;

        stringstream text;
        text<<"<text x=\""<<x<<"\" y=\""<<y<<"\" style=\"font-weight:bold\" font-size=\"30px\">"<<this->grid[i][j]<<"</text>\n";

        outFile << text.rdbuf();
      }
    }
  }

    outFile << "<text x=\"50\" y=\"500\" style=\"font-weight:bold\" font-size=\"15px\">Difficulty Level (0 being easiest): "                  <<this->difficultyLevel<<"</text>\n";
    outFile << "</svg>";

}
// END: Printing into SVG file


// START: Calculate branch difficulty score
//! Step 12 : Create a Difficulty Score which can calculate the odds of Sudoku Level Randomly
int Sudoku::branchDifficultyScore()
{
   int emptyPositions = -1;
   int tempGrid[9][9];
   int sum=0;

   for(int i=0;i<9;i++)
  {
    for(int j=0;j<9;j++)
    {
      tempGrid[i][j] = this->grid[i][j];
    }
  }

    while(emptyPositions!=0)
    {
    vector<vector<int> > empty;

    for(int i=0;i<81;i++)
    {
        if(tempGrid[(int)(i/9)][(int)(i%9)] == 0)
        {
          vector<int> temp;
    temp.push_back(i);

    for(int num=1;num<=9;num++)
    {
	    if(isSafe(tempGrid,i/9,i%9,num))
      {
	      temp.push_back(num);
	    }
	  }

	  empty.push_back(temp);
        }

      }

      if(empty.size() == 0)
      {
        cout<<"Hello: "<<sum<<endl;
        return sum;
      }

      int minIndex = 0;

      int check = empty.size();
      for(int i=0;i<check;i++)
      {
        if(empty[i].size() < empty[minIndex].size())
	      minIndex = i;
      }

      int branchFactor=empty[minIndex].size();
      int rowIndex = empty[minIndex][0]/9;
      int colIndex = empty[minIndex][0]%9;

      tempGrid[rowIndex][colIndex] = this->solnGrid[rowIndex][colIndex];
     sum = sum + ((branchFactor-2) * (branchFactor-2)) ;

      emptyPositions = empty.size() - 1;
    }

    return sum;

}
// END: Finish branch difficulty score


// START: Calculate difficulty level of current grid
void Sudoku::calculateDifficulty()
{
  int B = branchDifficultyScore();
  int emptyCells = 0;

  for(int i=0;i<9;i++)
  {
    for(int j=0;j<9;j++)
    {
	if(this->grid[i][j] == 0)
	    emptyCells++;
    }
  }

  this->difficultyLevel = B*100 + emptyCells;
}
// END: calculating difficulty level


// START: The main function
//! Step 13 : Create a main function
int main(int argc, char const *argv[])
{
  // Initialising seed for random number generation
  srand(time(NULL));

  // Creating an instance of Sudoku
  Sudoku *puzzle = new Sudoku();

  // Creating a seed for puzzle generation
  puzzle->createSeed();

  // Generating the puzzle
  puzzle->genPuzzle();

  // Calculating difficulty of puzzle
  puzzle->calculateDifficulty();

  // testing by printing the grid
  puzzle->printGrid();

  // Printing the grid into SVG file
  string rem = "sudokuGen";
  string path = argv[0];
  path = path.substr(0,path.size() - rem.size());
  puzzle->printSVG(path);
  cout<<"The above sudoku puzzle has been stored in puzzles.svg in current folder\n";
  // freeing the memory
  delete puzzle;

  return 0;
}
// END: The main function


