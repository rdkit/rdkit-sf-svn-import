//
//  Copyright (c) 2015 Greg Landrum
//
//   @@ All Rights Reserved @@
//  This file is part of the RDKit.
//  The contents are covered by the terms of the BSD license
//  which is included in the file license.txt, found at the root
//  of the RDKit source tree.
//
#include <GraphMol/GraphMol.h>
#include <GraphMol/QueryAtom.h>
#include <GraphMol/MolOps.h>
#include <GraphMol/QueryOps.h>
#include <GraphMol/AtomIterators.h>
#include <GraphMol/BondIterators.h>

#include <vector>
#include <algorithm>

namespace RDKit {
namespace {
bool isMapped(const Atom* atom) {
  return atom->hasProp(common_properties::molAtomMapNumber);
}

bool isRGroup(const Atom* atom) {
  return atom->hasProp(common_properties::_MolFileRLabel) || atom->getAtomicNum() == 0;
}

bool attachedToRGroup(const ROMol &mol, Atom*atom) {
  ROMol::ADJ_ITER nbrIdx, endNbrs;
  boost::tie(nbrIdx, endNbrs) = mol.getAtomNeighbors(atom);
  while (nbrIdx != endNbrs) {
    if (isRGroup(mol.getAtomWithIdx(*nbrIdx)))
      return true;
    ++nbrIdx;
  }
}

}

namespace MolOps {
ROMol *adjustQueryProperties(const ROMol &mol,
                             const AdjustQueryParameters *params) {
  RWMol *res = new RWMol(mol);
  try {
    adjustQueryProperties(*res, params);
  } catch (MolSanitizeException &se) {
    delete res;
    throw se;
  }
  return static_cast<ROMol *>(res);
}
void adjustQueryProperties(RWMol &mol, const AdjustQueryParameters *inParams) {
  AdjustQueryParameters params;
  if (inParams) {
    params = *inParams;
  }
  const RingInfo *ringInfo = mol.getRingInfo();

  if(params.aromatizeIfPossible) {
    unsigned int failed;
    sanitizeMol(mol, failed, SANITIZE_SYMMRINGS | SANITIZE_SETAROMATICITY);
  } else {
    if (!ringInfo->isInitialized()) {
      MolOps::symmetrizeSSSR(mol);
    }
  }
  
  for (unsigned int i = 0; i < mol.getNumAtoms(); ++i) {
    Atom *at = mol.getAtomWithIdx(i);
    // pull properties we need from the atom here, once we
    // create a query atom they may no longer be valid.
    unsigned int nRings = ringInfo->numAtomRings(i);
    int atomicNum = at->getAtomicNum();
    bool attachedR = attachedToRGroup(mol,at);
    if (params.adjustDegree &&
        !((params.adjustDegreeFlags & ADJUST_IGNORECHAINATOMS) && !nRings) &&
        !((params.adjustDegreeFlags & ADJUST_IGNORERINGATOMS) && nRings) &&
        !((params.adjustDegreeFlags & ADJUST_IGNOREDUMMIES) && !atomicNum) &&
        !((params.adjustDegreeFlags & ADJUST_IGNORENONDUMMIES) && atomicNum) &&
        !((params.adjustDegreeFlags & ADJUST_IGNOREMAPPED) && isMapped(at)) &&
        !((params.adjustDegreeFlags & ADJUST_IGNOREATTACHEDRGROUPS) && attachedR)
        ) {
      QueryAtom *qa;
      if (!at->hasQuery()) {
        qa = new QueryAtom(*at);
        mol.replaceAtom(i, qa);
        delete qa;
        qa = static_cast<QueryAtom *>(mol.getAtomWithIdx(i));
        at = static_cast<Atom *>(qa);
      } else {
        qa = static_cast<QueryAtom *>(at);
      }
      qa->expandQuery(makeAtomExplicitDegreeQuery(qa->getDegree()));
    }  // end of adjust degree
    if (params.adjustRingCount &&
        !((params.adjustRingCountFlags & ADJUST_IGNORECHAINATOMS) && !nRings) &&
        !((params.adjustRingCountFlags & ADJUST_IGNORERINGATOMS) && nRings) &&
        !((params.adjustRingCountFlags & ADJUST_IGNOREDUMMIES) && !atomicNum) &&
        !((params.adjustRingCountFlags & ADJUST_IGNORENONDUMMIES) &&
        !((params.adjustRingCountFlags & ADJUST_IGNOREMAPPED) && isMapped(at)) &&
        !((params.adjustRingCountFlags & ADJUST_IGNOREATTACHEDRGROUPS) && attachedR) &&
          atomicNum)) {
      QueryAtom *qa;
      if (!at->hasQuery()) {
        qa = new QueryAtom(*at);
        mol.replaceAtom(i, qa);
        delete qa;
        qa = static_cast<QueryAtom *>(mol.getAtomWithIdx(i));
        at = static_cast<Atom *>(qa);
      } else {
        qa = static_cast<QueryAtom *>(at);
      }
      qa->expandQuery(makeAtomInNRingsQuery(nRings));
    }  // end of adjust ring count
    if (params.makeDummiesQueries && atomicNum == 0 && !at->hasQuery() &&
        !at->getIsotope()) {
      QueryAtom *qa = new QueryAtom();
      qa->setQuery(makeAtomNullQuery());
      mol.replaceAtom(i, qa);
      delete qa;
      at = mol.getAtomWithIdx(i);
    }  // end of makeDummiesQueries
  }    // end of loop over atoms
}
}  // end of MolOps namespace
}  // end of RDKit namespace
