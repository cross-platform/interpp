/************************************************************************
Interpp - Light-Weight C++ Scripting Interpretor
Copyright (c) 2012-2013 Marcus Tomlinson

This file is part of Interpp.

The BSD 2-Clause License:
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
************************************************************************/

#ifndef INTERPP_H
#define INTERPP_H

#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <typeinfo>
#include <cstdlib>

//=================================================================================================

#define _INTERPP_REGISTER_METHOD_RETURN( Class, Method, ReturnType, ... )\
namespace Interpp\
{\
  static std::string _Call_##Class##_##Method( std::string& objectName, std::string& params )\
  {\
    Class* object = ( Class* ) ( _InterppRegistry::GetObject( objectName ) );\
    if( object )\
    {\
      ReturnType ( Class::*methPtr )( __VA_ARGS__ ) = &Class::Method;\
      return ConvertValue< std::string >( _Call_Method< Class, ReturnType, ##__VA_ARGS__ >( object, methPtr, params ) );\
    }\
    else\
    {\
      return "Error: object not found";\
    }\
  }\
\
  static void Init_##Class##_##Method()\
  {\
    _InterppRegistry::AddMethod< Class >( _Call_##Class##_##Method, #Method );\
  }\
}

//-------------------------------------------------------------------------------------------------

#define _INTERPP_REGISTER_METHOD_VOID( Class, Method, ... )\
namespace Interpp\
{\
  static std::string _Call_##Class##_##Method( std::string& objectName, std::string& params )\
  {\
    Class* object = ( Class* ) ( _InterppRegistry::GetObject( objectName ) );\
    if( object )\
    {\
      void ( Class::*methPtr )( __VA_ARGS__ ) = &Class::Method;\
      _Call_Method< Class, void, ##__VA_ARGS__ >( object, methPtr, params );\
      return "";\
    }\
    else\
    {\
      return "Error: object not found";\
    }\
  }\
\
  static void Init_##Class##_##Method()\
  {\
    _InterppRegistry::AddMethod< Class >( _Call_##Class##_##Method, #Method );\
  }\
}

//-------------------------------------------------------------------------------------------------

#define INTERPP_REGISTER_METHOD_RETURN( Class, Method, ReturnType, ... ) _INTERPP_REGISTER_METHOD_RETURN( Class, Method, ReturnType, ##__VA_ARGS__ )
#define INTERPP_REGISTER_METHOD_VOID( Class, Method, ... ) _INTERPP_REGISTER_METHOD_VOID( Class, Method, ##__VA_ARGS__ )

//=================================================================================================

namespace Interpp
{
  typedef std::string (*_interppMethod )( std::string&, std::string& );

  //-------------------------------------------------------------------------------------------------

  class _InterppRegistry
  {
  public:
    static void* GetObject( std::string& objectName )
    {
      _interppObjectsIt = _interppObjects.find( objectName );

      if( _interppObjectsIt == _interppObjects.end() )
      {
        return NULL;
      }

      return _interppObjectsIt->second.second;
    }

    template< class ObjectType >
    static void AddObject( void* object, std::string& objectName )
    {
      const std::type_info* objectType = &typeid( ObjectType );
      std::string objectTypeName = objectType->name();

      _interppObjects[ objectName ] = std::make_pair( objectTypeName, object );
    }

    static _interppMethod GetMethod( std::string& objectName, std::string& methodName )
    {
      _interppObjectsIt = _interppObjects.find( objectName );

      if( _interppObjectsIt == _interppObjects.end() )
      {
        return NULL;
      }

      methodName += _interppObjectsIt->second.first;
      _interppMethodsIt = _interppMethods.find( methodName );

      if( _interppMethodsIt == _interppMethods.end() )
      {
        return NULL;
      }

      return _interppMethodsIt->second;
    }

    template< class ObjectType >
    static void AddMethod( _interppMethod method, std::string methodName )
    {
      const std::type_info* objectType = &typeid( ObjectType );
      methodName += objectType->name();

      _interppMethods[ methodName ] = method;
    }

  private:
    static std::map< std::string, std::pair< std::string, void* > > _interppObjects;
    static std::map< std::string, std::pair< std::string, void* > >::const_iterator _interppObjectsIt;

    static std::map< std::string, _interppMethod > _interppMethods;
    static std::map< std::string, _interppMethod >::const_iterator _interppMethodsIt;
  };

  //-------------------------------------------------------------------------------------------------

  class _ParamList
  {
  public:
    _ParamList( std::string& params )
    {
      if( params.size() == 0 )
      {
        return;
      }

      unsigned long commaPos = 0;
      unsigned long paramStart = 0;
      unsigned long paramEnd = 0;

      while( commaPos != std::string::npos )
      {
        // skip spaces before param
        while( paramStart < params.size() - 1 &&
               params[paramStart] == ' ' )
        {
          paramStart++;
        }

        commaPos = paramStart;

        //if param is a string, skip to next inverted comma
        if( params[paramStart] == '\'' )
        {
          while( true )
          {
            commaPos = params.find( "'", commaPos + 1 );

            if( commaPos != std::string::npos &&
                params[commaPos - 1] == '\\' )
            {
              params.erase( commaPos - 1, 1 );
              commaPos--;
            }
            else
            {
              break;
            }
          }
        }

        // find end of current param
        commaPos = params.find( ",", commaPos );

        if( commaPos != std::string::npos )
        {
          paramEnd = commaPos;
        }
        else
        {
          paramEnd = params.size();
        }

        if( paramStart != paramEnd )
        {
          // skip spaces after param
          while( paramEnd > 0 &&
                 params[paramEnd - 1] == ' ' )
          {
            paramEnd--;
          }
        }

        // if the param is a string, copy contents within inverted commas
        if( ( params[paramStart] == '\'' || params[paramStart] == '\"' ) &&
            ( params[paramEnd-1] == '\'' || params[paramEnd-1] == '\"' ) )
        {
          paramStart++;
          paramEnd--;
        }

        // push param to params
        _params.push_back( params.substr( paramStart, paramEnd - paramStart ) );

        // start next param after comma
        paramStart = commaPos + 1;
      }
    }

    std::string operator []( unsigned long i ) const
    {
      if( i < _params.size() )
      {
        return _params[i];
      }

      return "";
    }

  private:
    std::vector< std::string > _params;
  };

  //-------------------------------------------------------------------------------------------------

  template< class ToType, class FromType >
  static ToType ConvertValue( FromType fromValue )
  {
    // convert from string
    // ===================
    if( typeid( FromType ) == typeid( std::string ) )
    {
      std::string fromString = *( ( std::string* ) ( &fromValue ) );

      // ToType is string
      if( typeid( ToType ) == typeid( std::string ) )
      {
        return *reinterpret_cast< ToType* >( &fromString );
      }
      // ToType is char*
      else if( typeid( ToType ) == typeid( char* ) )
      {
        return *reinterpret_cast< ToType* >( &fromString );
      }
      // ToType is char
      else if( typeid( ToType ) == typeid( char ) )
      {
        char returnValue = fromString[0];
        return *reinterpret_cast< ToType* >( &returnValue );
      }
      // ToType is unsigned char
      else if( typeid( ToType ) == typeid( unsigned char ) )
      {
        unsigned char returnValue = atoi( fromString.c_str() );
        return *reinterpret_cast< ToType* >( &returnValue );
      }
      // ToType is double
      else if( typeid( ToType ) == typeid( double ) )
      {
        double returnValue = atof( fromString.c_str() );
        return *reinterpret_cast< ToType* >( &returnValue );
      }
      // ToType is float
      else if( typeid( ToType ) == typeid( float ) )
      {
        float returnValue = ( float ) atof( fromString.c_str() );
        return *reinterpret_cast< ToType* >( &returnValue );
      }
      // ToType is int
      else if( typeid( ToType ) == typeid( int ) )
      {
        int returnValue = atoi( fromString.c_str() );
        return *reinterpret_cast< ToType* >( &returnValue );
      }
      // ToType is short
      else if( typeid( ToType ) == typeid( short ) )
      {
        short returnValue = ( short ) atoi( fromString.c_str() );
        return *reinterpret_cast< ToType* >( &returnValue );
      }
      // ToType is long
      else if( typeid( ToType ) == typeid( long ) )
      {
        long returnValue = atol( fromString.c_str() );
        return *reinterpret_cast< ToType* >( &returnValue );
      }
      // ToType is unsigned int
      else if( typeid( ToType ) == typeid( unsigned int ) )
      {
        unsigned int returnValue = atoi( fromString.c_str() );
        return *reinterpret_cast< ToType* >( &returnValue );
      }
      // ToType is unsigned short
      else if( typeid( ToType ) == typeid( unsigned short ) )
      {
        unsigned short returnValue = ( short ) atoi( fromString.c_str() );
        return *reinterpret_cast< ToType* >( &returnValue );
      }
      // ToType is unsigned long
      else if( typeid( ToType ) == typeid( unsigned long ) )
      {
        unsigned long returnValue = atol( fromString.c_str() );
        return *reinterpret_cast< ToType* >( &returnValue );
      }
      // ToType is bool
      else if( typeid( ToType ) == typeid( bool ) )
      {
        bool returnValue = fromString == "true";
        return *reinterpret_cast< ToType* >( &returnValue );
      }
    }

    // convert to string
    // =================
    else if( typeid( ToType ) == typeid( std::string ) )
    {
      std::ostringstream returnStream;

      // FromType is char*
      if( typeid( FromType ) == typeid( char* ) )
      {
        returnStream << *reinterpret_cast< char** >( &fromValue );
      }
      // FromType is char
      else if( typeid( FromType ) == typeid( char ) )
      {
        returnStream << *reinterpret_cast< char* >( &fromValue );
      }
      // FromType is unsigned char
      else if( typeid( FromType ) == typeid( unsigned char ) )
      {
        returnStream << *reinterpret_cast< unsigned short* >( &fromValue );
      }
      // FromType is double
      else if( typeid( FromType ) == typeid( double ) )
      {
        returnStream << *reinterpret_cast< double* >( &fromValue );
      }
      // FromType is float
      else if( typeid( FromType ) == typeid( float ) )
      {
        returnStream << *reinterpret_cast< float* >( &fromValue );
      }
      // FromType is int
      else if( typeid( FromType ) == typeid( int ) )
      {
        returnStream << *reinterpret_cast< int* >( &fromValue );
      }
      // FromType is short
      else if( typeid( FromType ) == typeid( short ) )
      {
        returnStream << *reinterpret_cast< short* >( &fromValue );
      }
      // FromType is long
      else if( typeid( FromType ) == typeid( long ) )
      {
        returnStream << *reinterpret_cast< long* >( &fromValue );
      }
      // FromType is unsigned int
      else if( typeid( FromType ) == typeid( unsigned int ) )
      {
        returnStream << *reinterpret_cast< unsigned int* >( &fromValue );
      }
      // FromType is unsigned short
      else if( typeid( FromType ) == typeid( unsigned short ) )
      {
        returnStream << *reinterpret_cast< unsigned short* >( &fromValue );
      }
      // FromType is unsigned long
      else if( typeid( FromType ) == typeid( unsigned long ) )
      {
        returnStream << *reinterpret_cast< unsigned long* >( &fromValue );
      }
      // FromType is bool
      else if( typeid( FromType ) == typeid( bool ) )
      {
        returnStream << ( *reinterpret_cast< bool* >( &fromValue ) ? "true" : "false" );
      }

      std::string returnValue = returnStream.str();
      return *reinterpret_cast< ToType* >( &returnValue );
    }

    return ToType();
  }

  //-------------------------------------------------------------------------------------------------

  template< class Cl, class Rt >
  static Rt _Call_Method( Cl* object, Rt ( Cl::*methPtr )(), _ParamList params )
  {
    return ( object->*methPtr )();
  }

  template< class Cl, class Rt, class T1 >
  static Rt _Call_Method( Cl* object, Rt ( Cl::*methPtr )( T1 ), _ParamList params )
  {
    return ( object->*methPtr )( ConvertValue< T1 >( params[0] ) );
  }

  template< class Cl, class Rt, class T1, class T2 >
  static Rt _Call_Method( Cl* object, Rt ( Cl::*methPtr )( T1, T2 ), _ParamList params )
  {
    return ( object->*methPtr )( ConvertValue< T1 >( params[0] ),
                                 ConvertValue< T2 >( params[1] ) );
  }

  template< class Cl, class Rt, class T1, class T2, class T3 >
  static Rt _Call_Method( Cl* object, Rt ( Cl::*methPtr )( T1, T2, T3 ), _ParamList params )
  {
    return ( object->*methPtr )( ConvertValue< T1 >( params[0] ),
                                 ConvertValue< T2 >( params[1] ),
                                 ConvertValue< T3 >( params[2] ) );
  }

  template< class Cl, class Rt, class T1, class T2, class T3, class T4 >
  static Rt _Call_Method( Cl* object, Rt ( Cl::*methPtr )( T1, T2, T3, T4 ), _ParamList params )
  {
    return ( object->*methPtr )( ConvertValue< T1 >( params[0] ),
                                 ConvertValue< T2 >( params[1] ),
                                 ConvertValue< T3 >( params[2] ),
                                 ConvertValue< T4 >( params[3] ) );
  }

  template< class Cl, class Rt, class T1, class T2, class T3, class T4, class T5 >
  static Rt _Call_Method( Cl* object, Rt ( Cl::*methPtr )( T1, T2, T3, T4, T5 ), _ParamList params )
  {
    return ( object->*methPtr )( ConvertValue< T1 >( params[0] ),
                                 ConvertValue< T2 >( params[1] ),
                                 ConvertValue< T3 >( params[2] ),
                                 ConvertValue< T4 >( params[3] ),
                                 ConvertValue< T5 >( params[4] ) );
  }

  template< class Cl, class Rt, class T1, class T2, class T3, class T4, class T5, class T6 >
  static Rt _Call_Method( Cl* object, Rt ( Cl::*methPtr )( T1, T2, T3, T4, T5, T6 ), _ParamList params )
  {
    return ( object->*methPtr )( ConvertValue< T1 >( params[0] ),
                                 ConvertValue< T2 >( params[1] ),
                                 ConvertValue< T3 >( params[2] ),
                                 ConvertValue< T4 >( params[3] ),
                                 ConvertValue< T5 >( params[4] ),
                                 ConvertValue< T6 >( params[5] ) );
  }

  template< class Cl, class Rt, class T1, class T2, class T3, class T4, class T5, class T6, class T7 >
  static Rt _Call_Method( Cl* object, Rt ( Cl::*methPtr )( T1, T2, T3, T4, T5, T6, T7 ), _ParamList params )
  {
    return ( object->*methPtr )( ConvertValue< T1 >( params[0] ),
                                 ConvertValue< T2 >( params[1] ),
                                 ConvertValue< T3 >( params[2] ),
                                 ConvertValue< T4 >( params[3] ),
                                 ConvertValue< T5 >( params[4] ),
                                 ConvertValue< T6 >( params[5] ),
                                 ConvertValue< T7 >( params[6] ) );
  }

  template< class Cl, class Rt, class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8 >
  static Rt _Call_Method( Cl* object, Rt ( Cl::*methPtr )( T1, T2, T3, T4, T5, T6, T7, T8 ), _ParamList params )
  {
    return ( object->*methPtr )( ConvertValue< T1 >( params[0] ),
                                 ConvertValue< T2 >( params[1] ),
                                 ConvertValue< T3 >( params[2] ),
                                 ConvertValue< T4 >( params[3] ),
                                 ConvertValue< T5 >( params[4] ),
                                 ConvertValue< T6 >( params[5] ),
                                 ConvertValue< T7 >( params[6] ),
                                 ConvertValue< T8 >( params[7] ) );
  }

  template< class Cl, class Rt, class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9 >
  static Rt _Call_Method( Cl* object, Rt ( Cl::*methPtr )( T1, T2, T3, T4, T5, T6, T7, T8, T9 ), _ParamList params )
  {
    return ( object->*methPtr )( ConvertValue< T1 >( params[0] ),
                                 ConvertValue< T2 >( params[1] ),
                                 ConvertValue< T3 >( params[2] ),
                                 ConvertValue< T4 >( params[3] ),
                                 ConvertValue< T5 >( params[4] ),
                                 ConvertValue< T6 >( params[5] ),
                                 ConvertValue< T7 >( params[6] ),
                                 ConvertValue< T8 >( params[7] ),
                                 ConvertValue< T9 >( params[8] ) );
  }

  template< class Cl, class Rt, class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10 >
  static Rt _Call_Method( Cl* object, Rt ( Cl::*methPtr )( T1, T2, T3, T4, T5, T6, T7, T8, T9, T10 ), _ParamList params )
  {
    return ( object->*methPtr )( ConvertValue< T1 >( params[0] ),
                                 ConvertValue< T2 >( params[1] ),
                                 ConvertValue< T3 >( params[2] ),
                                 ConvertValue< T4 >( params[3] ),
                                 ConvertValue< T5 >( params[4] ),
                                 ConvertValue< T6 >( params[5] ),
                                 ConvertValue< T7 >( params[6] ),
                                 ConvertValue< T8 >( params[7] ),
                                 ConvertValue< T9 >( params[8] ),
                                 ConvertValue< T10 >( params[9] ) );
  }

  //=================================================================================================

  static std::string Execute( std::string command )
  {
    std::string objectName;
    std::string methodName;
    std::string params;

    unsigned long findPos = 0;
    unsigned long lastFindPos = 0;

    // get object name
    findPos = command.find( ".", lastFindPos );

    if( findPos != std::string::npos )
    {
      objectName = command.substr( 0, findPos );
    }

    lastFindPos = findPos + 1;

    // get method name
    findPos = command.find( "(", lastFindPos );

    if( findPos != std::string::npos )
    {
      methodName = command.substr( lastFindPos, findPos - lastFindPos );
    }

    lastFindPos = findPos + 1;

    // get params
    findPos = command.find( ")", lastFindPos );

    if( findPos != std::string::npos )
    {
      params = command.substr( lastFindPos, findPos - lastFindPos );
    }

    // get method from registry
    _interppMethod method = _InterppRegistry::GetMethod( objectName, methodName );

    // execute method
    if( method != NULL )
    {
      return method( objectName, params );
    }
    else
    {
      return "Error: method not found";
    }
  }

  //-------------------------------------------------------------------------------------------------

  template< class Type >
  static void RegisterObject( Type& object, std::string objectName )
  {
    _InterppRegistry::AddObject< Type >( ( void* ) &object, objectName );
  }

  //-------------------------------------------------------------------------------------------------

  template< class Type >
  static void RegisterObject( Type* object, std::string objectName )
  {
    _InterppRegistry::AddObject< Type >( ( void* ) object, objectName );
  }
}

//=================================================================================================

#endif // INTERPP_H
