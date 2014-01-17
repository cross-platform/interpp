#include "interpp.h"
#include <iostream>
#include <stdio.h>

//=================================================================================================

class Simple
{
public:
  bool SameNumber( int num1, int num2 )
  {
    if( num1 != num2 )
    {
      return false;
    }

    return true;
  }

  void Print( std::string word, bool show )
  {
    if( show )
    {
      std::cout << word << " ";
    }
  };

  float Multiply( float x, float y )
  {
    return x * y;
  }

  virtual void Who()
  {
    std::cout << "parent";
  };
};

//-------------------------------------------------------------------------------------------------

class Simple2 : public Simple
{
  virtual void Who()
  {
    std::cout << "child";
  };
};

//-------------------------------------------------------------------------------------------------

// Register Methods
// ================
INTERPP_REGISTER_METHOD_RETURN( Simple, SameNumber, bool, int, int )
INTERPP_REGISTER_METHOD_VOID( Simple, Print, std::string, bool )
INTERPP_REGISTER_METHOD_RETURN( Simple, Multiply, float, float, float )
INTERPP_REGISTER_METHOD_VOID( Simple, Who )

//=================================================================================================

int main()
{
  // Init Methods
  // ============
  Interpp::Init_Simple_SameNumber();
  Interpp::Init_Simple_Print();
  Interpp::Init_Simple_Multiply();
  Interpp::Init_Simple_Who();

  // Expose Class Instances To Interpp
  // =================================
  Simple simple;
  Simple2 simple2;
  Interpp::RegisterObject( simple, "simple" );
  Interpp::RegisterObject( (Simple&)simple2, "simple2" );

  // Run Interactive Interpretor
  // ===========================
  std::cout << "Usage: simple.Multiply( 2, 5 )\n\n";

  std::string command;

  while( true )
  {
    getline( std::cin, command );

    if( command == "exit" )
    {
      break;
    }

    std::cout << Interpp::Execute( command ) << '\n';
  }

  return 0;
}

//=================================================================================================