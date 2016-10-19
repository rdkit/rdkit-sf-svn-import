//
// Writing molecules - example3.cpp

#include <iostream>
#include <string>
#include <GraphMol/SmilesParse/SmilesParse.h>
#include <GraphMol/SmilesParse/SmilesWrite.h>
#include <GraphMol/FileParsers/FileParsers.h>
#include <GraphMol/GraphMol.h>
#include <GraphMol/MolOps.h>

int main( int argc , char **argv ) {

   RDKit::ROMol *mol = RDKit::MolFileToMol( "data/chiral.mol" );
   std::cout << RDKit::MolToSmiles( *mol ) << std::endl;
   // 2nd parameter doIsomericSmiles defaults to false
   std::cout << RDKit::MolToSmiles( *mol , true ) << std::endl;

   RDKit::ROMol *mol1 = RDKit::SmilesToMol( "C1=CC=CN=C1" );
   std::cout << RDKit::MolToSmiles( *mol1 ) << std::endl;

   RDKit::ROMol *mol2 = RDKit::SmilesToMol( "c1cccnc1" );
   std::cout << RDKit::MolToSmiles( *mol2 ) << std::endl;

   RDKit::ROMol *mol3 = RDKit::SmilesToMol( "n1ccccc1" );
   std::cout << RDKit::MolToSmiles( *mol3 ) << std::endl;

   RDKit::RWMol *mol4 = new RDKit::RWMol( *mol );
   RDKit::MolOps::Kekulize( *mol4 );
   std::cout << RDKit::MolToSmiles( *mol4 ) << std::endl;

   delete mol1;
   mol1 = RDKit::SmilesToMol( "C1CCC1" );
   std::cout << RDKit::MolToMolBlock( *mol1 ) << std::endl;

   mol1->setProp( "_Name" , "cyclobutane" );
   std::cout << RDKit::MolToMolBlock( *mol1 ) << std::endl;
   
}
