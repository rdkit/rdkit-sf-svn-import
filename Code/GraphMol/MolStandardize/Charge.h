//
//  Copyright (C) 2018 Susan H. Leung
//
//   @@ All Rights Reserved @@
//  This file is part of the RDKit.
//  The contents are covered by the terms of the BSD license
//  which is included in the file license.txt, found at the root
//  of the RDKit source tree.
//
/*! \file Charge.h

	\brief Defines the Reionizer class and Uncharger class.

*/
#ifndef __RD_CHARGE_H__
#define __RD_CHARGE_H__

#include "MolStandardize.h"
#include <Catalogs/Catalog.h>
#include <GraphMol/MolStandardize/AcidBaseCatalog/AcidBaseCatalogEntry.h>
#include <GraphMol/MolStandardize/AcidBaseCatalog/AcidBaseCatalogParams.h>

namespace RDKit {
class RWMol;
class ROMol;

namespace MolStandardize {

extern const CleanupParameters defaultCleanupParameters;

typedef RDCatalog::HierarchCatalog<AcidBaseCatalogEntry, AcidBaseCatalogParams,
                                   int>
    AcidBaseCatalog;

struct ChargeCorrection {
  std::string Name;
  std::string Smarts;
  int Charge;

  ChargeCorrection(std::string name, std::string smarts, int charge)
      : Name(name), Smarts(smarts), Charge(charge) {}
};

// The default list of ChargeCorrections.
extern std::vector<ChargeCorrection> CHARGE_CORRECTIONS;

//! The reionizer class to fix charges and reionize a molecule such that the 
// strongest acids ionize first.
/*!

  <b>Notes:</b>
    - 
*/

class Reionizer {
 public:
	Reionizer();
	//! construct a Reionizer with a particular acidbaseFile
	Reionizer(const std::string acidbaseFile);
	//! construct a Reionizer with a particular acidbaseFile and charge
	//corrections
	Reionizer(const std::string acidbaseFile, const std::vector<ChargeCorrection> ccs);
	Reionizer(const Reionizer &other);
	~Reionizer();

	//! Enforce charges on certain atoms, then perform competitive reionization.
  ROMol *reionize(const ROMol &mol);

 private:
	AcidBaseCatalog *d_abcat;
	std::vector<ChargeCorrection> d_ccs;

  std::pair<unsigned int, std::vector<unsigned int>> *strongestProtonated(
      const ROMol &mol,
      const std::vector<std::pair<ROMOL_SPTR, ROMOL_SPTR>> &abpairs);
  std::pair<unsigned int, std::vector<unsigned int>> *weakestIonized(
      const ROMol &mol,
      const std::vector<std::pair<ROMOL_SPTR, ROMOL_SPTR>> &abpairs);

};  // Reionizer class

//! The Uncharger class for neutralizing ionized acids and bases.
/*!

  <b>Notes:</b>
    - This class uncharges molecules by adding and/or removing hydrogens.
	  - For zwitterions, hydrogens are moved to eliminate charges where possible.
	  - In cases where there is a positive charge that is not neutralizable, 
		an	attempt is made to also preserve the corresponding negative charge.

*/

class Uncharger {
 public:
  Uncharger();
  Uncharger(const Uncharger &other);
  ~Uncharger();

  ROMol *uncharge(const ROMol &mol);

 private:
  std::shared_ptr<ROMol> pos_h;
  std::shared_ptr<ROMol> pos_quat;
  std::shared_ptr<ROMol> neg;
  std::shared_ptr<ROMol> neg_acid;
};  // Uncharger class

}  // namespace MolStandardize
}  // namespace RDKit
#endif
