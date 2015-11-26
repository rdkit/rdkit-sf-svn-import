//
//  Copyright (C) 2015 Novartis Institutes for BioMedical Research
//
//   @@ All Rights Reserved @@
//  This file is part of the RDKit.
//  The contents are covered by the terms of the BSD license
//  which is included in the file license.txt, found at the root
//  of the RDKit source tree.
//
#include <map>
#include <list>
#include <vector>
#include <algorithm>
#include <math.h>

#include "../MolOps.h"
#include "../SmilesParse/SmilesParse.h"
//#include "../SmilesParse/SmilesWrite.h"
#include "../Substruct/SubstructMatch.h"
#include "ChemTransforms.h"

#include "RGroupDecomp.h"

namespace RDKit{ 

//typedef std::vector<ROMOL_SPTR>::const_iterator MolIterator;
//TODO:                          
typedef ROMOL_SPTR sidechain_t;

// Is it right ???????????? for res.back().Sidechains.sort();
bool operator < (const sidechain_t& left, const sidechain_t& right) {
    return left->getNumAtoms() < right->getNumAtoms();
}

static
void GetMolDegenPoints (const ROMol& mol, std::vector<unsigned>& equivPoints) {
    std::vector<MatchVectType> matches;
    SubstructMatch(mol, mol, matches, false);
    for(unsigned mIdx=0; mIdx < matches.size(); mIdx++) {
        for(unsigned idx=0; idx < matches[mIdx].size(); idx++) {
            if(matches[mIdx][idx].first != matches[mIdx][idx].second)
                equivPoints.push_back(matches[mIdx][idx].first);
            equivPoints.push_back(matches[mIdx][idx].second);
        }
    }
}

struct MolDegenPts {
    unsigned MolIdx;
    unsigned NumberDegenPoints;
    inline bool operator < (const MolDegenPts& right) { return NumberDegenPoints < right.NumberDegenPoints; }
    inline bool operator > (const MolDegenPts& right) { return NumberDegenPoints > right.NumberDegenPoints; }
};
inline bool operator < (const MolDegenPts& left, const MolDegenPts& right) { return left.NumberDegenPoints < right.NumberDegenPoints; }
inline bool operator > (const MolDegenPts& left, const MolDegenPts& right) { return left.NumberDegenPoints > right.NumberDegenPoints; }

struct MolSidechains {
    unsigned                 MolIdx;
    std::vector<sidechain_t> Sidechains;
    MolSidechains(unsigned i=0) : MolIdx(i) {}
};

struct DegenPtsChain {
    unsigned    DegenPtsIdx;
    sidechain_t DegenPtsSidechain;
    DegenPtsChain(unsigned i, const sidechain_t& chain) : DegenPtsIdx(i), DegenPtsSidechain(chain) {}
};

static
bool SymmetrizeSidechains (const std::vector<ROMOL_SPTR> &mols, const ROMol* core, 
                           const RGoupDecompositionOptions &options,  
                           const std::vector<std::vector<sidechain_t> > &sidechains,
                           std::vector<MolSidechains> &res) {
    if(mols.size() != sidechains.size()) {
        if(options.Verbose)
            std::cout<<"ERROR: sidechains list must be as long as molecules list\n";
        return false;
    }
    if(NULL == core) {
// TODO: build copy from sidechains
//        res = sidechains;
        return true;
    }
    std::vector<MatchVectType> matches;
    SubstructMatch(*core, *core, matches, false);
    if(matches.size() <= 1) {
// TODO: build copy from sidechains
//        res = sidechains;
        return true;
    }

    std::vector<unsigned> degenPts;
    GetMolDegenPoints(*core, degenPts);

//  # start by ordering the molecules by the number of attachment points on degenerate positions:
    std::vector<MolDegenPts> newOrder(mols.size());
    for(size_t i=0; i < mols.size(); i++) {
        unsigned nDegen =0; // the number of attachment points
        for(size_t idx = 0; idx < sidechains[i].size(); idx++)
            if(degenPts.end() != std::find(degenPts.begin(), degenPts.end(), idx))
                nDegen++;
        newOrder[i].NumberDegenPoints = nDegen; // append((nDegen,i))
        newOrder[i].MolIdx = (unsigned) i;
    }
    std::sort(newOrder.begin(), newOrder.end(), std::greater<MolDegenPts>());

    res.clear();
    size_t i=0;
//  # all the mols without chains in degenerate positions:
    for(i=0; i < newOrder.size() && 0 == newOrder[i].NumberDegenPoints; i++) {
        unsigned molIdx = newOrder[i].MolIdx;
        res.push_back(MolSidechains(molIdx));
        res.back().Sidechains = sidechains[molIdx];
//TODO: sort operator <
//        res.back().Sidechains.sort();
    }
//  # all the mols with single chains in degenerate positions:
    for( ; i < newOrder.size() && 1 == newOrder[i].NumberDegenPoints; i++) {
        unsigned molIdx = newOrder[i].MolIdx;
//      # assign the degenerate positions the lowest index:
        std::vector<unsigned> tChainsIdx;
        unsigned idx = -1;
        for(size_t tIdx=0; tIdx < sidechains[molIdx].size(); tIdx++) {
            const sidechain_t& tChain = sidechains[molIdx][tIdx];
            if(degenPts.end() == std::find(degenPts.begin(), degenPts.end(), tIdx)) // not exists
                tChainsIdx.push_back((unsigned) tIdx);
            else
                idx = (unsigned) tIdx;
        }
        if(-1 == idx)
            return false; // assert idx is not None //?????
        unsigned minId = idx;
        
        for(size_t mi=0; mi < matches.size(); mi++) // for matchId,match in enumerate(matches):
            if(matches[mi][idx].first < (int)minId)
                minId = matches[mi][idx].first;   // ?????????? first() ???????????????
        tChainsIdx.push_back(minId);
        res.push_back(MolSidechains(molIdx));   /// res.append((molIdx,tuple(tChains)))
        for(size_t si=0; si < tChainsIdx.size(); si++)
            res.back().Sidechains.push_back(sidechains[molIdx][si]);
// TODO:
//        res.back().Sidechains.sort();
// TODO:
//        chainLocCount[chain][minId]+=1
//        posChainLists[minId].append(chain)
    }
//  # the remaining molecules have more than one chain in a degenerate position
    for( ; i < newOrder.size(); ) {
        const unsigned count = newOrder[i].NumberDegenPoints;
        for( ; i < newOrder.size() && newOrder[i].NumberDegenPoints == count; i++) {
            unsigned molIdx = newOrder[i].MolIdx;
/*        
            localStates=[]
            seen = set()
            for(size_t mi=0; mi < matches.size(); mi++) { // for matchId,match in enumerate(matches):
                chainState = []
        
                //for idx,chain in sidechains[molIdx]:
                for(size_t chain_idx=0; chain_idx < sidechains[molIdx].size(); chain_idx++) {
                  pIdx = matches[mi][chain_idx];  ///???  = match[idx]
                  //chainState.append((idx in degenPts,-chainLocCount[chain][pIdx],pIdx,chain))
                  chainState.push_back(...(chain_idx in degenPts, -chainLocCount[sidechains[molIdx][chain_idx]] [pIdx]
                                    , pIdx, sidechains[molIdx][chain_idx]);
                }
                chainState.sort()
                localStates.append(chainState)
            }
*/

//            localStates.sort();
            
            
/*

            winner = localStates[0]
            tChains=[]
            for a,b,idx,chain in winner:
            tChains.append((idx,chain))
            chainLocCount[chain][idx]+=1
            posChainLists[idx].append(chain)
            tChains.sort()
            res.append((molIdx,tuple(tChains)))
*/
        }
    }    
/*
  res.sort()
  res = [y for x,y in res]
  
  return res
*/
    return true;
}

static
void GetSidechains(const std::vector<ROMOL_SPTR> &mols, const std::vector<ROMOL_SPTR> &cores, 
                   const RGoupDecompositionOptions &options,  
                   std::vector< std::vector< sidechain_t > > &res) {
    
    for(size_t molIdx=0; molIdx < mols.size(); molIdx++) {
        if(NULL == mols[molIdx]) { //never
            /*
//TODO:                          
                  localRes =[(None,'bad molecule')]*len(cores)
                  res.append(tuple(localRes))
            */
            continue;
        }
        const ROMol& mol = *mols[molIdx];
        bool molMatched = false;
//    localRes =[]
        for(size_t i=0; i < cores.size(); i++) {
            if(NULL == cores[i]) {
//TODO:                          
                //localRes.append((None,'bad core'));
                continue;
            }
            const ROMol& core = *cores[i];

//TODO:            std::vector< .... > coreRes;
            std::vector<MatchVectType> tmatches;
            SubstructMatch(mol, core, tmatches);
            if(tmatches.empty()) {
                if(options.Verbose)
                    std::cout<<"molecule "<<molIdx<<" did not match core "<<i<<"\n";
//TODO:                          
//                coreRes=[(None,'no core matches')]
            }
            else if(tmatches.size() > 1) {
                if(options.Verbose)
                    std::cout<<"core "<<i<<" matches molecule multiple times\n";
//TODO:                          
//                coreRes=[(None,'core matches multiple times')]
            }
            else {
                std::auto_ptr<ROMol> tMol(replaceCore(mol, core, true, true));
                if(tMol->getNumAtoms() > 0){
                    molMatched = true;
                    //Fragment tMol:    //tSmi = Chem.MolToSmiles(tMol,True)   tSmi = tSmi.split('.') :
                    std::vector< std::vector<int> > frags;
                    unsigned nFrags = MolOps::getMolFrags(*tMol, frags);
                    for(size_t fi=0; fi < frags.size(); fi++) { // for elem in tSmi:
                        std::map<unsigned, unsigned> newAtomMap;
                        unsigned matches = 0;    // number of attachment points
                        unsigned idx = 0;
                        RWMol* sideChain = new RWMol(); // == cSmi / elem
                        for(size_t ai=0; ai < frags[i].size(); ai++) {
                            const Atom* a = tMol->getAtomWithIdx(frags[fi][ai]);
                            int label =0;
                            a->getPropIfPresent(common_properties::molAtomMapNumber, label);
                            if(0 == a->getAtomicNum() && 0!=label ) // attachment point like [1*] smarts
                            {
                                matches++;  // matches = dummyExpr.findall(elem)   dummyExpr=re.compile(r'\[([0-9]*)\*\]')
                                if( 1 == matches) // first attachment point
                                    idx = frags[fi][ai]; // or ai ???????????????    // ==  idx = matches[0]
                            }
                            else {  // add atom to sidechain (all atoms except attachment points)
                                    // elem = dummyExpr.sub('*',elem)    // REMOVE Labled attachment point [*] ???????
// Is it right ????????????
                                newAtomMap[frags[fi][ai]] = sideChain->addAtom(a->copy(), true, true);
                            }
                        }
                        //add bonds between added atoms
                        std::map<unsigned, unsigned> visitedBonds;
                        for(size_t ai=0; ai < frags[i].size(); ai++) {
                            Atom* a = tMol->getAtomWithIdx(frags[fi][ai]);
                            ROMol::OEDGE_ITER beg,end;
                            for(boost::tie(beg,end) = tMol->getAtomBonds(a); beg!=end; ++beg){
                                const BOND_SPTR bond = (*tMol)[*beg];
                                if(newAtomMap.end() == newAtomMap.find(bond->getBeginAtomIdx())
                                || newAtomMap.end() == newAtomMap.find(bond->getEndAtomIdx())
                                || visitedBonds.end() != visitedBonds.find(bond->getIdx()) )
                                    continue;
                                unsigned ai1 = newAtomMap.at(bond->getBeginAtomIdx());
                                unsigned ai2 = newAtomMap.at(bond->getEndAtomIdx());
                                unsigned bi  = sideChain->addBond(ai1, ai2, bond->getBondType());
                                visitedBonds[bond->getIdx()] = bi;
                            }
                        }
                        
                        if (0 == matches)
                            if (options.Verbose)
                                std::cout << "sidechain with no attachment point label: fi="<<fi<<"\n";
                        else { 
                            if(matches > 1)
                                if(options.RejectDoubleAttachments) {
//TODO:                          
                                    //coreRes=((None,'multiple attachment points in sidechain'),)
                                    break;
                                }
                                else
                                    if(options.Verbose)
                                        std::cout<<"sidechain with multiple attachment point labels: fi="<<fi<<"\n";
/* SKIP:  ???????????????????                            
                          try:
                            /// convert  sideChain to sideChain by AvalonTools  --- SKIP for binary ROMol
                            cSmi = pyAvalonTools.GetCanonSmiles(elem,True)
                          except:
                            pass
                          else:
                            elem = cSmi
*/
//TODO:                          
//                        coreRes.append((idx, ROMOL_SPTR(sideChain)); // attachment point index and fragment/core ?????????????
                        }
                    }                  
                }
            }  
//TODO:                          
            //localRes.append(tuple(coreRes))
        }    
        if( ! molMatched && options.Verbose)
            std::cout<<"molecule "<<molIdx<<" did not match any cores\n";
    }
//TODO:                          
    //res.append(tuple(localRes))
    
}

static
void ProcessCoreLabels(const std::vector<ROMOL_SPTR> &cores, const RGoupDecompositionOptions &options, 
                             std::vector<ROMOL_SPTR> &resultCores) {
   

    for(size_t coreIdx=0; coreIdx < cores.size(); coreIdx++) {
        std::vector<unsigned> rmv;
        const ROMol& core = *cores[coreIdx];
        const size_t coreNumAtoms = core.getNumAtoms();
        for(size_t i=0; i < coreNumAtoms; i++)
        {

            const Atom* coreAtom = core.getAtomWithIdx((unsigned)i);
            if(0 == coreAtom->getAtomicNum()) { // == core.GetSubstructMatches(Chem.MolFromSmiles('[*]'))
                //# use the isotope we read in to set the R label.
                //# this is appropriate because the mol file parser sets R group
                //# labels the same way.
                //# if we come in as SMARTS we use the atom mapping facility
                int label=-1;
                if (coreAtom->hasProp(common_properties::molAtomMapNumber))
                    coreAtom->getProp(common_properties::molAtomMapNumber, label);
                else
                    label = coreAtom->getIsotope();

                if(0 == coreAtom->getDegree())  // attachment point has no neighbors
                    continue;
        //      if len(nbrs)>1: //        logger.warning('attachment point has more than one neighbor')
                ROMol::ADJ_ITER begin,end;
                
                boost::tie(begin,end) = core.getAtomNeighbors(coreAtom);  //nbrs = dummy.GetNeighbors()
                for( ; begin!=end; begin++) { //nbr = nbrs[0]
                    //coreAtomNeighbors.push_back(core->getAtomWithIdx(*begin));
                    std::string str;
                    str.append("%d", label);
                    core.getAtomWithIdx((unsigned)*begin)->setProp("_RLabel", str); //nbr
        //      labelDict[nbr.GetIdx()] = label
                    rmv.push_back((unsigned)*begin);
                    break; // ??? nbr = nbrs[0]
                }
            }

        }

        RWMol* em = new RWMol(core);
        std::sort(rmv.begin(), rmv.end(), std::greater<unsigned>());  // reverse sort
        for(std::vector<unsigned>::const_iterator ri = rmv.begin(); ri != rmv.end(); ri++)
          em->removeAtom(*ri);
        resultCores.push_back(ROMOL_SPTR(em));
        //em._labelDict=labelDict
    }
}



//=====================================================================
// Public API implementation:
//=====================================================================

void RGroupDecomposite(const std::vector<ROMOL_SPTR> &mols, const std::vector<ROMOL_SPTR> &srcCores, const RGoupDecompositionOptions &options, 
                             std::vector<ROMOL_SPTR> &results)
{

    std::vector<ROMOL_SPTR>  processedCores;
    const std::vector<ROMOL_SPTR>* cores = NULL;

    std::vector<MatchVectType> coreCoreMatches(cores->size());  /// ??? 
    if(options.LabelledCores) {
    ProcessCoreLabels(*cores, options, processedCores);
    cores = &processedCores;
    std::vector<MatchVectType> matches;
    for(size_t i=0; i < cores->size(); i++) {
        SubstructMatch(*(*cores)[i], *(*cores)[i], matches, false);
        if(matches.empty()) // # pathology happens at times with query features in the core
            for(int ai=0; ai < (int)(*cores)[i]->getNumAtoms(); ai++)
                coreCoreMatches[i].push_back(std::pair<int,int>(ai,0)); // ??? matches=(tuple(range(core.GetNumAtoms())),)
        else
            for(unsigned j=0; j < matches.size(); j++)
                for(unsigned k=0; k < matches[j].size(); k++)
                    coreCoreMatches[i].push_back(matches[j][k]); // append
        }
    }
    else
        cores = &srcCores;

    std::vector< std::vector< sidechain_t > > sidechains;  // res: set of Sidechains for each core
    GetSidechains(mols, *cores, options, sidechains);
    
    if(options.Symmetrize) {
        if(options.Verbose)
            std::cout<<"Symmetrizing R groups\n";
  
        for(size_t coreIdx=0; coreIdx < cores->size(); coreIdx++) {
            std::vector< MolSidechains > sChains;
/*
            SymmetrizeSidechains(mols, (*cores)[coreIdx], options, sidechains[coreIdx], sChains);

    //// ????????????????????????????????????????????????????????????????? :
            for(size_t i=0; i < sidechains.size(); i++) {  //for i,row in enumerate(res):
                row = list(row)
                row[coreIdx] = sChains[i]
                coreSidechains[i] = row
            }
*/
        }
    }
  
    if(options.LabelledCores) {
        if(options.Verbose)
            std::cout<<"Renumbering to match input ordering\n";
        for(size_t coreIdx=0; coreIdx < cores->size(); coreIdx++) {
            const ROMol& core = *((*cores)[coreIdx]);
//      # for symmetric cores where we're labelling, we have to deal with the real PITA
//      # of a core like *C1CC1. Since the dummy gets subsumed into its attached atom
//      # when we do the replacement, this could match C1CC1F three different ways. The one
//      # we get isn't determined by logic, but by atom ordering. We want to present
//      # a sensible result, so we have to do some work:
/*
        const MatchVectType& ccM = coreCoreMatches[coreIdx];
        sidechain_t chains(results[coreIdx].size())
        for(size_t i=0; i < chains.size(); i++)   // chains = [x[coreIdx] for x in res]
            chains[i] = results[coreIdx][i];

      for i,row in enumerate(res):
        possibleMappings=[0]*len(ccM)  /////???????????????????????????????????????????
        chain = chains[i]
        chain = list(chain)
        for cidx,(idx,smi) in enumerate(chain):
          if idx is None:
            res[i] = (idx,smi)
            chain=None
            break
          for midx,mapping in enumerate(ccM):
            if core.GetAtomWithIdx(mapping[idx]).HasProp('_RLabel'):
              #print '    ',cidx,midx,idx,'->',mapping[idx]
              possibleMappings[midx] += 1
        if chain is not None:
//          #print 'POSS:',possibleMappings
          best = sorted(zip(possibleMappings,range(len(possibleMappings))))
          #raise ValueError(best)
          best = best[-1][1]
          bestMapping = ccM[best]
//          #print 'BEST:',bestMapping

          for cidx,(idx,smi) in enumerate(chain):
            if not core.GetAtomWithIdx(bestMapping[idx]).HasProp('_RLabel'):
              if options.requireLabels or options.nonLabelledSubstituentHandling=='FAIL':
                if not options or not options.silent:
                  logger.warning('skipping molecule %d because it has a substitution at an unmarked position'%i)
                res[i] = (None,'substitution at unmarked position')
                chain=None
                break
              elif options.nonLabelledSubstituentHandling=='IGNORE':
                idx=-1
              else:
                idx+=100
            else:
              idx=int(core.GetAtomWithIdx(bestMapping[idx]).GetProp('_RLabel'))
            if idx>=0:
              chain[cidx]=(idx,smi)
            else:
              chain[cidx]=None
        row = list(row)
        if chain is not None:
          chain = tuple([x for x in chain if x is not None])
          row[coreIdx]=chain
          res[i] = row
*/
        }
//// return mols,res
    }  
}

 
} // end of namespace RDKit

