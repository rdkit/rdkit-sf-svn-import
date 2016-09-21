# $Id: test_MurckoScaffold.py 3672 2010-06-14 17:10:00Z landrgr1 $
#
# Created by Peter Gedeck, June 2008
#
from rdkit.Chem.Scaffolds.MurckoScaffold import *

import unittest
import random
from rdkit import Chem

class TestCase(unittest.TestCase):
  testMolecules = [
    ("CC1CCC1", "C1CCC1"),
    ("NCNCC2CC2C1CC1O", "C1CC1C1CC1"),
    ("OC2C(C)C21C(N)C1C", "C2CC12CC1"),  # Spiro
    ("C1CC1C(=O)OC", "C1CC1"),  # Carbonyl outside scaffold
    ("C1CC1C=C", "C1CC1"),  # Double bond outside scaffold
    ("C1CC1C=CC1CC1C=CNNCO", "C1CC1C=CC1CC1"),  # Double bond in scaffold
    ("CC1CC1C(N)C1C(N)C1", "C1CC1CC1CC1"),
    ("C1CC1S(=O)C1CC1C=CNNCO", "C1CC1S(=O)C1CC1"),  # S=O group in scaffold
    ("O=SCNC1CC1S(=O)C1CC1C=CNNCO", "C1CC1S(=O)C1CC1"),  # S=O group outside scaffold
    ("C1CC1S(=O)(=O)C1CC1C=CNNCO", "C1CC1S(=O)(=O)C1CC1"),  # SO2 group in scaffold
    ("O=S(CNCNC)(=O)CNC1CC1S(=O)(=O)C1CC1C=CNNCO", "C1CC1S(=O)(=O)C1CC1"),  # noqa SO2 group outside scaffold
    ("C1CC1C=NO", "C1CC1"),  # Hydroxamide
    ("C1CC1C(C(C)C)=NC1CC1", "C1CC1C=NC1CC1"),  # Hydroxamide
    ("C1CC1C#N", "C1CC1"),  # Cyano group
    ("C1CC1C#CNC", "C1CC1"),  # Acetylene group
    ("O=C1N(C)C(=O)N1C#CNC", "O=C1NC(=O)N1"),  # Acetylene group
    ("[O-][N+](=O)c1cc(ccc1Cl)NS(=O)(=O)Cc2ccccc2", "c1ccccc1NS(=O)(=O)Cc2ccccc2"),
    ("Cn1cccc1", "c1ccc[nH]1"),
    ("C1CC1[CH](C)C1CC1", "C1CC1CC1CC1"),
    ]

  testMolecules2 = [
    ("CCOc1ccccc1N(S(C)(=O)=O)CC(NC1CCCCC1)=O", "O=C(NC1CCCCC1)CNc1ccccc1"),
    ("c1ccc(-c2c(C)n(-c3c(C(O)=O)cccc3)c(C)nc2=O)cc1", "O=c1c(cn(cn1)-c1ccccc1)-c1ccccc1"),
    ("Cc1ccc(Cl)c2c1NC(=O)C2=C1NC(=S)NC1=O", "c1cc2c(cc1)C(=C1C(NC(N1)=S)=O)C(=O)N2"),
    ("CNC(=O)CCc1[nH]c2c(c1Sc1ccccc1)cccc2", "c1cc(Sc2c3c([nH]c2)cccc3)ccc1"),
    ("CC(=O)OCC(=O)C1(O)CCC2C1(C)CC(=O)C1C3(C)CCC(=O)C=C3CCC21", "O=C1C=C2CCC3C4CCCC4CC(=O)C3C2CC1"),  # noqa
    ("CC(C)CC(Nc1nc(Cl)ccc1[N+]([O-])=O)C(O)=O", "c1ccncc1"),
    ("COc1ccc(C(Nc2ccc(S(N3C(C)CCCC3)(=O)=O)cc2)=O)c(OC)c1OC", "O=C(Nc1ccc(S(=O)(=O)N2CCCCC2)cc1)c1ccccc1"),  # noqa
    ("CC(C)CCNc1nc(N)c([N+](=O)[O-])c(NCCO)n1", "c1cncnc1"),
    ("c1ccc(Oc2c(NC(COC(c3c(C)noc3C)=O)=O)cccc2)cc1", "O=C(COC(=O)c1cnoc1)Nc1ccccc1Oc1ccccc1"),
    ("COC(CCCCC1SCC(NC(OC)=O)C1NC(OC)=O)=O", "C1CCCS1"),
    ("CSc1ccc(-c2c(C#N)c(N)nc3n(-c4ccccc4)nc(C)c32)cc1", "c1ccc(cc1)-c1c2c(n(nc2)-c2ccccc2)ncc1"),
    ("O=C1Cc2ccccc2Sc2c1cc(Cl)cc2", "O=C1Cc2ccccc2Sc2ccccc21"),
    ("COC(c1n(CC(N(C)c2ccccc2)=O)c2ccsc2c1)=O", "O=C(Cn1c2ccsc2cc1)Nc1ccccc1"),
    ("N=C1C(=Cc2coc3ccccc3c2=O)C(=O)N=C2SC(c3ccncc3)=NN12", "N=C1C(=Cc2coc3ccccc3c2=O)C(=O)N=C2SC(c3ccncc3)=NN12"),  # noqa
    ("CCOC(c1ccc(NC(CCc2c(C)nc3ncnn3c2C)=O)cc1)=O", "O=C(Nc1ccccc1)CCc1cnc2n(ncn2)c1"),
    ("COC(=O)C1=C(C)NC(C)=C(C(OC)=O)C1c1oc(-c2c(Cl)c(Cl)ccc2)cc1", "c1ccc(-c2oc(C3C=CNC=C3)cc2)cc1"),  # noqa
    ("CCN(S(c1cc(NC(COC(CCc2nc3ccccc3s2)=O)=O)ccc1)(=O)=O)CC", "c1cc(NC(COC(=O)CCc2nc3c(s2)cccc3)=O)ccc1"),  # noqa
    ("CCOC(c1cc(OC(c2ccccc2)=O)n(-c2ccccc2)n1)=O", "O=C(Oc1n(ncc1)-c1ccccc1)c1ccccc1"),
    ("CCOC(=O)c1nc2c(c(NCc3ccccc3F)n1)cccc2", "c1ccc(CNc2ncnc3c2cccc3)cc1"),
    ("Cc1nc(C)n(CC(N2CCCC(C(c3c(C)cc(Cl)cc3)=O)C2)=O)n1", "c1ccc(cc1)C(=O)C1CCCN(C(=O)Cn2cncn2)C1"),
    ("COc1cc(NC(=O)c2nnn(CCc3ccccc3)c2N)c(OC)cc1", "O=C(c1nnn(c1)CCc1ccccc1)Nc1ccccc1"),
    ("Cc1cc(C(=O)CN2C(=O)c3ccccc3C2=O)c(C)n1Cc1cccs1", "O=C(CN1C(c2c(cccc2)C1=O)=O)c1cn(Cc2cccs2)cc1"),  # noqa
    ("c1cnc2c(c1)cccc2S(N1CCC(C(=O)N2CCN(c3ccc(Cl)cc3)CC2)CC1)(=O)=O", "c1ccc(cc1)N1CCN(C(=O)C2CCN(S(=O)(=O)c3c4ncccc4ccc3)CC2)CC1"),  # noqa
    ("CCOC(c1c(C)[nH]c(C(NNC(c2ccc(C(C)(C)C)cc2)=O)=O)c1C)=O", "c1ccc(cc1)C(NNC(c1ccc[nH]1)=O)=O"),
    ("CCOC(c1cc(C(C)C)sc1NC(=O)COC(CCS(c1ccccc1)(=O)=O)=O)=O", "c1ccc(S(CCC(=O)OCC(Nc2cccs2)=O)(=O)=O)cc1"),  # noqa
    ("CCC1CCCCN1CCCNC(=O)Cn1nc(-c2ccccc2)ccc1=O", "O=C(NCCCN1CCCCC1)Cn1nc(ccc1=O)-c1ccccc1"),
    ("CCc1cc(OCCn2nc(C(O)=O)c3ccccc3c2=O)ccc1", "O=c1n(CCOc2ccccc2)ncc2ccccc21"),
    ("Fc1ccc(CN2CCN3C(CCC3)C2C2CCCCC2)cc1F", "c1ccc(cc1)CN1CCN2CCCC2C1C1CCCCC1"),
    ("O=[N+]([O-])c1cc(-c2nnc(N3CCOCC3)c3ccccc23)ccc1N1CCOCC1", "c1cc2c(nnc(c2cc1)N1CCOCC1)-c1ccc(cc1)N1CCOCC1"),  # noqa
    ("Cc1ccnc(NC(=O)COc2ccc3oc4c(c3c2)CCCC4)c1", "O=C(COc1ccc2oc3c(c2c1)CCCC3)Nc1ccccn1"),
    ("Cc1cc(=O)oc(C)c1C(=O)NCCCN1CCN(c2ccc(F)cc2)CC1", "c1ccc(N2CCN(CCCNC(c3ccc(oc3)=O)=O)CC2)cc1"),  # noqa
    ("Cc1cc(C(=O)CSc2nc(=O)cc(N)[nH]2)c(C)n1-c1cccc(F)c1", "O=C(CSc1nc(cc[nH]1)=O)c1cn(cc1)-c1ccccc1"),  # noqa
    ("CCN(S(c1cccc(C(=O)N2CCCCC2)c1)(=O)=O)CC", "O=C(N1CCCCC1)c1ccccc1"),  # noqa
    ("CNC(=S)N1CCC(NC(=O)C23CC4CC(C2)CC(C3)C4)CC1", "O=C(NC1CCNCC1)C12CC3CC(C1)CC(C3)C2"),  # noqa
    ("Cc1cc2c(cc1)N=C(C)C(N=O)=C(C)N2", "c1cc2NC=CC=Nc2cc1"),  # noqa
    ("COc1ccc(Sc2cc(C(F)(F)F)nc(-c3ncccc3)n2)cc1", "c1ccc(cc1)Sc1nc(ncc1)-c1ncccc1"),  # noqa
    ("c1coc(CNC(Cn2cc(C(c3ccccc3)=O)c3c2cccc3)=O)c1", "c1coc(CNC(Cn2cc(C(c3ccccc3)=O)c3c2cccc3)=O)c1"),  # noqa
    ("O=C(NCc1ccc(Cl)cc1)c1noc(-c2ccco2)c1", "O=C(c1noc(c1)-c1ccco1)NCc1ccccc1"),  # noqa
    ("CN(C)c1ccc(C(c2n(CCOC(=O)Nc3ccc(Cl)cc3)nnn2)N2CCOCC2)cc1", "O=C(Nc1ccccc1)OCCn1nnnc1C(c1ccccc1)N1CCOCC1"),  # noqa
    ("NC(=NOC(=O)c1cc(Cn2cc(C(F)(F)F)ccc2=O)ccc1)c1ccccc1", "c1ccc(C=NOC(c2cc(Cn3ccccc3=O)ccc2)=O)cc1"),  # noqa
    ("CCc1nnc(NC(=O)Cc2c(-c3ccc(C)cc3)nc(C)s2)s1", "O=C(Cc1c(-c2ccccc2)ncs1)Nc1nncs1"),  # noqa
    ("COCCCNC(=O)CN1C(=O)N(Cc2ccccc2Cl)CC1", "O=C1NCCN1Cc1ccccc1"),  # noqa
    ("Cc1cc([N+]([O-])=O)nn1CC(=O)NCCCn1ccnc1", "O=C(Cn1nccc1)NCCCn1ccnc1"),  # noqa
    ("c1cc(F)c(N2CCN(C(=O)c3ccc(S(NCC4OCCC4)(=O)=O)cc3)CC2)cc1", "c1ccc(cc1)N1CCN(C(c2ccc(cc2)S(=O)(=O)NCC2OCCC2)=O)CC1"),  # noqa
    ("CC(NCc1cccnc1)=C1C(=O)NC(=O)N(c2ccc(C)cc2)C1=O", "c1cc(ccc1)N1C(=O)NC(C(=CNCc2cccnc2)C1=O)=O"),  # noqa
    ("Cc1ccn(C)c(=N)c1", "N=c1[nH]cccc1"),  # noqa
    ("Cc1cc(C)nc(N2CCC(CNC(=O)CCc3ccccc3)CC2)n1", "O=C(CCc1ccccc1)NCC1CCN(c2ncccn2)CC1"),  # noqa
    ("CCOC1=CC(=CNNC(CCCC(NC2CCCCC2)=O)=O)C=CC1=O", "C1=CC(C=CC1=O)=CNNC(=O)CCCC(=O)NC1CCCCC1"),  # noqa
    ("CC(=O)N1CCN(c2ccc([N+]([O-])=O)cc2)CC1", "c1ccc(cc1)N1CCNCC1"),  # noqa
    ("CS(N(CC(=O)N1CCCCC1)Cc1ccc(Cl)cc1)(=O)=O", "O=C(N1CCCCC1)CNCc1ccccc1"),  # noqa
    ("c1coc(C(=O)N2CCN(C(COc3cc(C(NCc4ccccc4)=O)ccc3)=O)CC2)c1", "c1coc(C(=O)N2CCN(C(COc3cc(C(NCc4ccccc4)=O)ccc3)=O)CC2)c1"),  # noqa
    ("Cc1cccc2sc(NNC(=O)C3=COCCO3)nc12", "O=C(NNc1nc2ccccc2s1)C1=COCCO1"),  # noqa
    ("c1ccc2c(c1)N(C)C1(C=Nc3c(cc(N4CCOCC4)c4ccccc34)O1)C2(C)C", "C1=Nc2c(cc(c3ccccc23)N2CCOCC2)OC11Nc2ccccc2C1"),  # noqa
    ("COc1cccc(C2N(CCN3CCOCC3)C(=O)C(O)=C2C(=O)c2sc(C)nc2C)c1", "O=C(C1=CC(=O)N(C1c1ccccc1)CCN1CCOCC1)c1scnc1"),  # noqa
    ("COc1cc(OC)c(NC(CSc2nc3c(c(=O)n2-c2ccc(F)cc2)SCC3)=O)cc1", "c1ccc(cc1)NC(=O)CSc1n(c(=O)c2c(n1)CCS2)-c1ccccc1"),  # noqa
    ("Cc1ccccc1CN1c2ccccc2C2(C1=O)OCCCO2", "O=C1C2(OCCCO2)c2c(N1Cc1ccccc1)cccc2"),  # noqa
    ("O=C(N1C2(OCC1)CCN(c1ncc(C(F)(F)F)cc1Cl)CC2)c1ccccc1", "O=C(c1ccccc1)N1C2(OCC1)CCN(c1ccccn1)CC2"),  # noqa
    ("CC=CC=CC(=O)Nc1nccs1", "c1ncsc1"),  # noqa
    ("CC(C)(C)c1ccc(C(c2c[nH]c(C(NCc3cccnc3)=O)c2)=O)cc1", "c1ccc(cc1)C(=O)c1c[nH]c(c1)C(=O)NCc1cccnc1"),  # noqa
    ("CCC(=O)Nc1c(C)nn(-c2cc(C)c(C)cc2)c1C", "c1ccc(cc1)-n1nccc1"),  # noqa
    ("Cc1ccc(SCCC(=O)NCCSCc2c(C)cccc2)cc1", "O=C(NCCSCc1ccccc1)CCSc1ccccc1"),  # noqa
    ("CC1=NN(Cc2ccccc2)C(=O)C1=Cc1ccc(N(C)C)cc1", "O=C1C(C=NN1Cc1ccccc1)=Cc1ccccc1"),  # noqa
    ("COCC(=O)Nc1ccc(S(NCCc2ccccc2)(=O)=O)cc1", "c1ccc(CCNS(=O)(=O)c2ccccc2)cc1"),  # noqa
    ("CCOC(=O)N(C)c1ccc(C(O)(C(F)(F)F)C(F)(F)F)cc1", "c1ccccc1"),  # noqa
    ("Fc1ccc(COC2=C(C(O)=O)CCNC2=O)cc1F", "O=C1NCCC=C1OCc1ccccc1"),  # noqa
    ("O=C1N2C(Nc3ccccc31)CCCCC2", "O=C1N2C(Nc3ccccc31)CCCCC2"),  # noqa
    ("Cl.COc1ccc(-c2nc3n(ccc4ccccc43)c2CN2CCOCC2)cc1OC", "c1cccc(c1)-c1nc2c3c(ccn2c1CN1CCOCC1)cccc3"),  # noqa
    ("ClCc1oc(-c2ccccc2)nn1", "c1oc(nn1)-c1ccccc1"),  # noqa
    ("Cl.Cc1ccc(OCC(O)Cn2c(=N)n(CCN3CCCCC3)c3ccccc32)cc1", "N=c1n(CCCOc2ccccc2)c2ccccc2n1CCN1CCCCC1"),  # noqa
    ("COc1ccc(C(=O)C=C(C)Nc2ccc3c(c2)OCO3)cc1", "O=C(C=CNc1ccc2c(c1)OCO2)c1ccccc1"),  # noqa
    ("c1csc(CN(C(c2ccc(F)cc2)C(NC2CCCCC2)=O)C(=O)CN2S(=O)(=O)c3ccccc3C2=O)c1", "c1cc(CN(C(=O)CN2S(=O)(c3ccccc3C2=O)=O)C(C(=O)NC2CCCCC2)c2ccccc2)sc1"),  # noqa
    ("c1csc(S(NCCSc2n(-c3ccccc3)nnn2)(=O)=O)c1", "c1csc(S(NCCSc2n(-c3ccccc3)nnn2)(=O)=O)c1"),  # noqa
    ("Cc1cccc(C=NNC(=O)Cn2c(N)nnn2)n1", "O=C(Cn1cnnn1)NN=Cc1ccccn1"),  # noqa
    ("CCOC(C1(Cc2ccc(Cl)cc2)CCN(C(c2cc(C)nc(C)n2)=O)CC1)=O", "O=C(N1CCC(CC1)Cc1ccccc1)c1ccncn1"),  # noqa
    ("c1ccc(C(N(CC2OCCC2)C(Cn2nnc3ccccc23)=O)C(NCc2ccc(F)cc2)=O)cc1", "O=C(N(C(c1ccccc1)C(=O)NCc1ccccc1)CC1OCCC1)Cn1nnc2c1cccc2"),  # noqa
    ("O=C1CSC(c2ccncc2)N1Cc1occc1", "O=C1CSC(c2ccncc2)N1Cc1occc1"),  # noqa
    ("COc1c(OCc2ccccc2)c(Br)cc(C=NNC(=O)Cn2nc([N+]([O-])=O)cc2C)c1", "O=C(Cn1nccc1)NN=Cc1ccc(cc1)OCc1ccccc1"),  # noqa
    ("Cc1c(Cn2nnc(-c3cc(C(=O)O)ccc3)n2)cccc1", "c1cccc(-c2nn(nn2)Cc2ccccc2)c1"),  # noqa
    ("O=C(c1ccc2snnc2c1)N1CCCC1", "O=C(c1ccc2snnc2c1)N1CCCC1"),  # noqa
    ("c1ccc(CC(NN2C(=O)C(=Cc3c(C(O)=O)cccc3)SC2=S)=O)cc1", "O=C1C(=Cc2ccccc2)SC(=S)N1NC(Cc1ccccc1)=O"),  # noqa
    ("Cc1ccccc1OCC(=O)NN=Cc1ccncc1", "O=C(COc1ccccc1)NN=Cc1ccncc1"),  # noqa
    ("O=C(C=Cc1ccccc1)NC(=S)Nc1ccc(CN2CCOCC2)cc1", "O=C(C=Cc1ccccc1)NC(=S)Nc1ccc(CN2CCOCC2)cc1"),  # noqa
    ("COc1ccc(NC(=S)N(Cc2cnccc2)Cc2c(=O)[nH]c3c(c2)cc(OC)c(OC)c3)cc1", "O=c1c(CN(C(=S)Nc2ccccc2)Cc2cnccc2)cc2ccccc2[nH]1"),  # noqa
    ("Nc1ccc2nc3c([nH]c(=O)n(C4CCCCC4)c3=O)nc2c1", "c1ccc2nc3[nH]c(n(c(c3nc2c1)=O)C1CCCCC1)=O"),  # noqa
    ("Cc1cc(NC(=O)c2ccc(S(Nc3ccccc3)(=O)=O)cc2)no1", "c1cc(no1)NC(=O)c1ccc(S(=O)(=O)Nc2ccccc2)cc1"),  # noqa
    ("Nn1c(Cc2c3c(cccc3)ccc2)nnc1SCc1ccccc1", "c1ccc(CSc2nnc([nH]2)Cc2c3c(cccc3)ccc2)cc1"),  # noqa
    ("Cc1[nH]nc(Nc2cc(C)ccc2)c1[N+](=O)[O-]", "c1ccc(cc1)Nc1n[nH]cc1"),  # noqa
    ("CC1Cn2c(nc3n(C)c(=O)[nH]c(=O)c23)O1", "O=c1[nH]c2nc3n(c2c([nH]1)=O)CCO3"),  # noqa
    ("c1csc(C(OCC(NC23CC4CC(C2)CC(C3)C4)=O)=O)c1", "c1csc(C(OCC(NC23CC4CC(C2)CC(C3)C4)=O)=O)c1"),  # noqa
    ("c1ccc(S(NC2=NC(=O)C(=Cc3cnccc3)S2)(=O)=O)cc1", "c1ccc(S(NC2=NC(=O)C(=Cc3cnccc3)S2)(=O)=O)cc1"),  # noqa
    ("CCCn1c(N2CCN(C)CC2)nc2n(C)c(=O)[nH]c(=O)c12", "O=c1[nH]c([nH]c2nc([nH]c12)N1CCNCC1)=O"),  # noqa
    ("CCn1c(SCC(Nc2cc(S(N3CCOCC3)(=O)=O)ccc2OC)=O)nnc1-c1ccncc1", "c1cc(S(=O)(=O)N2CCOCC2)cc(NC(=O)CSc2nnc(-c3ccncc3)[nH]2)c1"),  # noqa
    ("C#CCNC(=O)C1=CC(c2ccc(Br)cc2)CC(OCc2ccc(CO)cc2)O1", "c1cccc(c1)C1C=COC(OCc2ccccc2)C1"),  # noqa
    ("CCc1c(SCC(=O)Nc2cc(C)on2)nc2ccc(C)cc2c1", "O=C(Nc1ccon1)CSc1ccc2c(cccc2)n1"),  # noqa
    ("CCOCCCN(C(C(NC1CCCC1)=O)c1cccc(OC)c1OC)C(c1ccco1)=O", "c1cc(ccc1)C(NC(c1occc1)=O)C(=O)NC1CCCC1"),  # noqa
    ("Cc1ccc(C(=O)NC(=S)NNS(c2ccccc2)(=O)=O)cc1", "c1cccc(c1)C(NC(=S)NNS(=O)(=O)c1ccccc1)=O"),  # noqa
    ("COc1ccc(CC(N)=NOC(=O)c2sccc2)cc1", "O=C(ON=CCc1ccccc1)c1sccc1"),  # noqa
    ("c1ccc(C(O)=C2C(c3ncccc3)N(CC(OC)OC)C(=O)C2=O)cc1", "c1cc(C=C2C(=O)C(=O)NC2c2ncccc2)ccc1"),  # noqa
    ("COC(=O)CSc1nc(C)cc(Oc2ccccc2)n1", "c1ccc(Oc2ccncn2)cc1"),  # noqa
    ("COc1ccc(Cn2c(C)ccc2C)cc1", "c1ccc(cc1)Cn1cccc1"),  # noqa
    ("COc1cccc(N2CCN(C3CC(=O)N(c4ccc(C)c(Cl)c4)C3=O)CC2)c1", "O=C1N(c2ccccc2)C(=O)C(C1)N1CCN(c2ccccc2)CC1"),  # noqa
    ("COc1cccc(OC)c1OCCN(C)C.OC(=O)C(O)=O", "c1ccccc1"),  # noqa
    ("C1CCC(NC(=O)c2ccc(S(N3CCCC3)(=O)=O)cc2)C1", "C1CCC(NC(=O)c2ccc(S(N3CCCC3)(=O)=O)cc2)C1"),  # noqa
    ("CCCN(C(=O)Cn1ncc2c(=O)oc3c(c12)cccc3)c1cc(C)ccc1", "O=C(Cn1ncc2c(oc3c(cccc3)c12)=O)Nc1ccccc1"),  # noqa
    ("CNC(NC(CSc1nnc(C(F)(F)F)n1C)=O)=O", "n1nc[nH]c1"),  # noqa
    ("CCOCCCN1C(=O)CC(C(NCCc2ccc(C)cc2)=O)C1", "O=C1NCC(C1)C(NCCc1ccccc1)=O"),  # noqa
    ("COc1c([N+](=O)[O-])cc(CSc2n[nH]c(C)n2)cc1", "c1ccc(CSc2nc[nH]n2)cc1"),  # noqa
    ("CN(C)CC(=O)c1ccc(-c2ccccc2)cc1", "c1cccc(c1)-c1ccccc1"),  # noqa
    ("CC1(O)C(=O)c2c(cccc2)N(c2ccccc2)C1=O", "O=C1CC(=O)N(c2c1cccc2)c1ccccc1"),  # noqa
    ("CN(S(c1ccccc1)(=O)=O)CC(=O)NCCc1ccccc1", "c1ccc(CCNC(=O)CNS(=O)(=O)c2ccccc2)cc1"),  # noqa
    ("CCNc1ccccc1C(=O)O", "c1ccccc1"),  # noqa
    ("CC1(C)C(CSc2nc3ccccc3[nH]2)C1(Cl)Cl", "c1ccc2c(nc([nH]2)SCC2CC2)c1"),  # noqa
    ("CC(C)c1ccc(OCC(=O)NC(=S)Nc2c3cccc4c3c(cc2)CC4)cc1", "O=C(NC(=S)Nc1c2cccc3c2c(cc1)CC3)COc1ccccc1"),  # noqa
    ("CN(C)c1ccc(NC(CN2CCC(C(c3ccc(F)cc3)=O)CC2)=O)cc1", "c1cccc(c1)NC(CN1CCC(CC1)C(=O)c1ccccc1)=O"),  # noqa
    ("CCCCN(C)C(=O)Cc1c(OC)ccc2cc(Br)ccc21", "c1c2ccccc2ccc1"),  # noqa
    ("Cc1ccc(NC(CSc2sc(NC(CN3CCOCC3)=O)nn2)=O)cc1", "O=C(Nc1ccccc1)CSc1sc(nn1)NC(=O)CN1CCOCC1"),  # noqa
    ("COCCNC(=S)NNc1cccc(C(=O)O)c1", "c1ccccc1"),  # noqa
    ("O=C(CNc1ccccc1)NN=Cc1ccc2c(c1)OCCO2", "O=C(CNc1ccccc1)NN=Cc1ccc2c(c1)OCCO2"),  # noqa
    ("COc1cc2ccccc2cc1C(=O)NCC(c1sccc1)N(C)C", "O=C(NCCc1sccc1)c1cc2c(cc1)cccc2"),  # noqa
    ("COc1ccc(C(N(C)C)CNC(=O)CCOc2ccccc2)cc1", "O=C(NCCc1ccccc1)CCOc1ccccc1"),  # noqa
    ("Cl.CCN(CC)CCCN1C(=O)CSC1c1ccc([N+]([O-])=O)cc1", "O=C1CSC(c2ccccc2)N1"),  # noqa
    ("CCC(Nc1ccc(OC)cc1OC)=C1C(=O)NC(=O)NC1=O", "c1cc(NC=C2C(=O)NC(=O)NC2=O)ccc1"),  # noqa
    ("c1coc(-c2cc(C(F)(F)F)nc(NCc3ccc(F)cc3)n2)c1", "c1ccc(CNc2nccc(n2)-c2occc2)cc1"),  # noqa
    ("CCOC(Nc1sc(C)c(C)c1C(OCC)=O)=O", "c1ccsc1"),  # noqa
    ("O=CN1CCN(C(C(=O)NC2CCCCC2)c2cc3c(cc2[N+]([O-])=O)OCO3)CC1", "O=C(C(N1CCNCC1)c1ccc2c(c1)OCO2)NC1CCCCC1"),  # noqa
    ("COc1cc(C2N(c3ccc(Br)cc3)C(=O)c3n[nH]c(C)c32)ccc1O", "O=C1c2n[nH]cc2C(N1c1ccccc1)c1ccccc1"),  # noqa
    ("c1cc(NC(=O)c2ccccc2[N+]([O-])=O)c(N2CCOCC2)cc1", "O=C(Nc1c(cccc1)N1CCOCC1)c1ccccc1"),  # noqa
    ("N#Cc1cc2c(nc1SCC(=O)N1CCCCC1)CCCCC2", "O=C(N1CCCCC1)CSc1ccc2c(n1)CCCCC2"),  # noqa
    ("CCN(CC)c1ccc(CN(C(=O)c2cc(OC)c(OC)c(OC)c2)C2CCS(=O)(=O)C2)cc1", "O=S1(=O)CCC(N(Cc2ccccc2)C(=O)c2ccccc2)C1"),  # noqa
    ("COc1cc(NC(=S)N2CCN(Cc3ccccc3)CC2)cc(OC)c1", "S=C(N1CCN(CC1)Cc1ccccc1)Nc1ccccc1"),  # noqa
    ("CC(=O)C(=CNc1ccc(OCc2ccccc2)cc1)c1ccccc1", "c1cccc(c1)COc1ccc(NC=Cc2ccccc2)cc1"),  # noqa
    ("CC(C)C(C(NC(C)C(N)=O)=O)NC(C1CCCN1C(OC(C)(C)C)=O)=O", "C1CCNC1"),  # noqa
    ("CCOc1ccc(N2CC(C(=O)Nc3cccc(S(NC4=NCCC4)(=O)=O)c3)CC2=O)cc1", "c1cccc(c1)N1CC(C(=O)Nc2cccc(S(=O)(=O)NC3=NCCC3)c2)CC1=O"),  # noqa
    ("O=C(NCc1ccccc1Cl)CSc1ccc(-c2cccs2)nn1", "O=C(NCc1ccccc1)CSc1ccc(nn1)-c1sccc1"),  # noqa
    ("COc1ccc(OC)c(N=c2ssnc2Cl)c1", "c1cccc(c1)N=c1ssnc1"),  # noqa
    ("CC(=O)C1=C(C)NC(=O)CC1c1c(Cl)cccc1", "O=C1CC(C=CN1)c1ccccc1"),  # noqa
    ("CCC(=O)N=C(N)Nc1nc(C)c2cc(C)c(C)cc2n1", "c1cc2c(cc1)ncnc2"),  # noqa
    ("Cc1ccccc1C(OC1OC(=O)C(Cl)=C1Nc1ccc(C(O)=O)cc1)=O", "O=C(OC1OC(C=C1Nc1ccccc1)=O)c1ccccc1"),  # noqa
    ("CCOc1cc(CN2CCC(CO)(Cc3cccc(C(F)(F)F)c3)CC2)ccc1OC", "c1ccc(cc1)CC1CCN(Cc2ccccc2)CC1"),  # noqa
    ("Cc1cc2c([nH]c(=O)c(CCNC(c3cccs3)=O)c2)cc1C", "O=C(NCCc1cc2ccccc2[nH]c1=O)c1cccs1"),  # noqa
    ("Cc1ccc(Nc2cc(=O)[nH]c(=O)[nH]2)cc1C", "c1cccc(c1)Nc1cc([nH]c([nH]1)=O)=O"),  # noqa
    ("Cc1cc(OCC(=O)NC2CCS(=O)(=O)C2)c2c(oc(=O)c3c2CCC3)c1", "O=C(NC1CCS(=O)(C1)=O)COc1c2c(ccc1)oc(c1c2CCC1)=O"),  # noqa
    ("CCc1sc(NC(CCC(NCCc2ccc(OC)c(OC)c2)=O)=O)nn1", "c1cc(ccc1)CCNC(=O)CCC(=O)Nc1scnn1"),  # noqa
    ("N#CC1=C(SCc2ccccc2)NC(=O)CC1c1ccc(O)cc1", "O=C1NC(=CC(C1)c1ccccc1)SCc1ccccc1"),  # noqa
    ("O=C(NCCN1CCOCC1)c1csc2c1CCCC2", "O=C(NCCN1CCOCC1)c1csc2c1CCCC2"),  # noqa
    ("CCCCC(=O)Nc1cc(OC)c(NC(C2CCCCC2)=O)cc1OC", "O=C(Nc1ccccc1)C1CCCCC1"),  # noqa
    ("Cc1ccc(C(C(C)OC(C2CC(=O)N(C3CCCCC3)C2)=O)=O)cc1", "c1cc(C(=O)COC(C2CC(=O)N(C2)C2CCCCC2)=O)ccc1"),  # noqa
    ("Cc1ccc(S(C(C#N)c2c(N3CCCC3)nc3ccccc3n2)(=O)=O)cc1C", "c1ccc(cc1)S(=O)(=O)Cc1c(nc2ccccc2n1)N1CCCC1"),  # noqa
    ("CC1(C)OC(=O)C(=Cc2[nH]ccc2)C(=O)O1", "O=C1OCOC(=O)C1=Cc1[nH]ccc1"),  # noqa
    ("Cc1cc(C)cc(Oc2nc3n(cccc3C)c(=O)c2C=C(C#N)C(=O)NC2CCS(=O)(=O)C2)c1", "c1ccc(cc1)Oc1c(c(=O)n2ccccc2n1)C=CC(=O)NC1CCS(=O)(=O)C1"),  # noqa
    ("COc1cc(NC(=O)NCc2c(C)onc2-c2ccccc2)ccc1", "O=C(NCc1conc1-c1ccccc1)Nc1ccccc1"),  # noqa
    ("c1ccc(C(Oc2cc3c(cc2)C(=O)CO3)=O)cc1", "c1ccc(C(Oc2cc3c(cc2)C(=O)CO3)=O)cc1"),  # noqa
    ("CCN1C(=O)C2C(c3cccs3)N3C4C(=O)N(CC)C(=O)C4C(c4cccs4)N3C2C1=O", "c1cc(sc1)C1C2C(NC(=O)C2N2N1C1C(=O)NC(=O)C1C2c1cccs1)=O"),  # noqa
    ("Cc1cc(C(N2CCCC(C(c3cc(F)ccc3F)=O)C2)=O)c(C)o1", "O=C(N1CCCC(C(=O)c2ccccc2)C1)c1cocc1"),  # noqa
    ("COc1cc(C=NO)ccc1Oc1c([N+]([O-])=O)cc([N+]([O-])=O)cc1", "c1cccc(Oc2ccccc2)c1"),  # noqa
    ("Cc1ccc(N(Cc2c(=O)[nH]c3ccc(C)cc3c2)C(c2cccs2)=O)cc1", "O=C(N(c1ccccc1)Cc1c([nH]c2c(cccc2)c1)=O)c1cccs1"),  # noqa
    ("COc1ccc(C(=O)Nn2c(C)nnc2-n2c(C)cc(C)n2)cc1OC", "O=C(c1ccccc1)Nn1cnnc1-n1nccc1"),  # noqa
    ("Cc1c(NC(=O)c2c(C)c(Cl)c(C)nc2Cl)cccc1", "O=C(c1cccnc1)Nc1ccccc1"),  # noqa
    ("c1ccc(CNC(CC(C(=O)NCc2ccccc2)c2nc(=O)c3ccccc3[nH]2)=O)cc1", "c1ccc(CNC(CC(C(=O)NCc2ccccc2)c2nc(=O)c3ccccc3[nH]2)=O)cc1"),  # noqa
    ("CNc1n(-c2ccccc2)ncc1[N+](=O)[O-]", "c1n(ncc1)-c1ccccc1"),  # noqa
    ("CC1SC2(NC1=O)C1CC3CC(C1)CC2C3", "O=C1CSC2(N1)C1CC3CC(C1)CC2C3"),  # noqa
    ("CCc1ccccc1NC(=S)N(C(C)c1occc1)CCOC", "S=C(NCc1occc1)Nc1ccccc1"),  # noqa
    ("CCC(C)NC(=O)C1CCCN(S(c2ccc(-n3cnnn3)cc2)(=O)=O)C1", "C1CCN(CC1)S(=O)(=O)c1ccc(cc1)-n1nnnc1"),  # noqa
    ("COc1c2c(ccc1)C1CC(C)(O2)N(Cc2ccccc2)C(=O)N1", "O=C1NC2CC(Oc3ccccc32)N1Cc1ccccc1"),  # noqa
    ("COc1ccc(C2NC(=O)c3c(cccc3)O2)c(OC)c1OC", "O=C1NC(Oc2c1cccc2)c1ccccc1"),  # noqa
    ("O=C(NNC=C1C=Nc2ccccc21)c1ccn(Cc2c(Cl)cc(Cl)cc2)n1", "O=C(NNC=C1c2c(cccc2)N=C1)c1nn(cc1)Cc1ccccc1"),  # noqa
    ("c1ccc(NS(c2ccc(OCC(=O)NCc3cnccc3)cc2)(=O)=O)cc1", "c1ccc(NS(c2ccc(OCC(=O)NCc3cnccc3)cc2)(=O)=O)cc1"),  # noqa
    ("COC1=CC(=O)C(=C2NNC(C(F)(F)F)=C2c2cc3ccccc3o2)C=C1", "O=C1C=CC=CC1=C1NNC=C1c1cc2ccccc2o1"),  # noqa
    ("CCOC(=O)c1c(C(COC(C=Cc2ccc(Cl)cc2)=O)=O)c(C)[nH]c1C", "c1ccc(C=CC(OCC(=O)c2cc[nH]c2)=O)cc1"),  # noqa
    ("Cc1nc2ncnn2c(N2CCN(c3nnnn3-c3ccccc3)CC2)c1", "c1nc2ncnn2c(c1)N1CCN(c2nnnn2-c2ccccc2)CC1"),  # noqa
    ("CC(C)Oc1ccc(C(=O)Nc2ccc(NC(c3ccco3)=O)c(Cl)c2)cc1", "O=C(Nc1ccc(cc1)NC(=O)c1ccccc1)c1occc1"),  # noqa
    ("CC(c1ccccc1)NC(C(NCC1OCCC1)=O)=O", "O=C(NCc1ccccc1)C(=O)NCC1OCCC1"),  # noqa
    ("CCCCOc1ccc(NC(=O)CCSc2nccn2C)cc1", "O=C(Nc1ccccc1)CCSc1ncc[nH]1"),  # noqa
    ("O=C(OCc1ncccc1)c1oc(COc2c(Cl)cccc2)cc1", "O=C(OCc1ncccc1)c1ccc(o1)COc1ccccc1"),  # noqa
    ("COc1ccc(C=NNC(=O)OC(C)(C)C)cc1OC", "c1ccccc1"),  # noqa
    ("CC1CCCCC1NC(COC(c1ccc(S(NCc2ccco2)(=O)=O)cc1)=O)=O", "c1coc(c1)CNS(=O)(=O)c1ccc(cc1)C(=O)OCC(=O)NC1CCCCC1"),  # noqa
    ("Nn1c(SCC(=O)Nc2cccc(F)c2)nnc1C1CCCCC1", "O=C(CSc1[nH]c(nn1)C1CCCCC1)Nc1ccccc1"),  # noqa
    ("Cc1n[nH]c(NC2CCCCC2)nc1=O", "O=c1cn[nH]c(n1)NC1CCCCC1"),  # noqa
    ("CCCCCCCCC(=O)NC(C(Cl)(Cl)Cl)NC(=S)N1CCOCC1", "C1NCCOC1"),  # noqa
    ("CCCc1ccc(Oc2coc3cc(OCC(Nc4c(C)cccc4)=O)ccc3c2=O)cc1", "c1cccc(c1)Oc1c(c2ccc(cc2oc1)OCC(=O)Nc1ccccc1)=O"),  # noqa
    ("Cc1ccc(C(=O)NN=C2CCSC2)cc1[N+]([O-])=O", "O=C(NN=C1CCSC1)c1ccccc1"),  # noqa
    ("N#CC1=C2SCN(c3ccc(F)cc3)CN2C(=O)CC1c1cc(F)ccc1", "O=C1N2CN(c3ccccc3)CSC2=CC(c2ccccc2)C1"),  # noqa
    ("c1ccc(CN2C(=O)CC(Nc3cc4c(cc3)cccc4)C2=O)cc1", "c1ccc(CN2C(=O)CC(Nc3cc4c(cc3)cccc4)C2=O)cc1"),  # noqa
    ("COc1ccc(NC(C)=O)cc1NC(=O)CN1CCN(CC(=O)Nc2ccc(Cl)cc2)CC1", "O=C(Nc1ccccc1)CN1CCN(CC1)CC(=O)Nc1ccccc1"),  # noqa
    ("Clc1c(Cl)c(C2NC(=O)CCC2[N+]([O-])=O)ccc1", "O=C1NC(CCC1)c1ccccc1"),  # noqa
    ("CCN(C(=O)CSc1n(-c2ccccc2)c(-c2ccccc2)nn1)CC", "c1ccc(cc1)-n1cnnc1-c1ccccc1"),  # noqa
    ("CC(=O)CCCCn1cnc2n(C)c(=O)n(C)c(=O)c12", "O=c1[nH]c(c2c(nc[nH]2)[nH]1)=O"),  # noqa
    ("CC1=NN(c2ccccc2)C(=N)C1=NNc1ccc(Cl)cc1", "N=C1C(=NNc2ccccc2)C=NN1c1ccccc1"),  # noqa
    ("CCc1ccc(OCC(=O)N(CC)CC)cc1", "c1ccccc1"),  # noqa
    ("CN(CC(=O)N1CCCCC1)S(c1ccc(Cl)cc1)(=O)=O", "O=C(CNS(=O)(=O)c1ccccc1)N1CCCCC1"),  # noqa
    ("CSc1ncc(C=C2C(=O)NC(=O)N(c3ccc(C)cc3)C2=O)cn1", "c1ccc(N2C(NC(=O)C(=Cc3cncnc3)C2=O)=O)cc1"),  # noqa
    ("COCCNC(=S)Nc1c(Cc2ccccc2)cccc1", "c1ccc(Cc2ccccc2)cc1"),  # noqa
    ("COc1cc(C(=O)Nc2nnc(C(C)(C)C)s2)c([N+]([O-])=O)cc1OC", "O=C(Nc1nncs1)c1ccccc1"),  # noqa
    ("CCOC(=O)c1ccc(NC(=O)c2cc(OC)c(OC(C)C)cc2)cc1", "O=C(Nc1ccccc1)c1ccccc1"),  # noqa
    ("COc1ccc(C(=O)C=C2Sc3cc4c(cc3N2C)OCO4)cc1", "O=C(C=C1Sc2cc3c(cc2N1)OCO3)c1ccccc1"),  # noqa
    ("CCCC1=NN(c2sc3c(n2)cccc3)C(=O)C1=CNCCCN(CC)CC", "C=C1C=NN(C1=O)c1sc2ccccc2n1"),  # noqa
    ("COc1ccc(C(COC(CN2C(=O)NC(C)(C)C2=O)=O)=O)cc1OC", "c1ccc(C(=O)COC(=O)CN2C(=O)CNC2=O)cc1"),  # noqa
    ("O=C(Oc1ccc(Br)cc1)C1CC(=O)N(c2ccc(F)cc2)C1", "O=C(C1CC(N(C1)c1ccccc1)=O)Oc1ccccc1"),  # noqa
    ("O=c1nc(-c2ccccn2)[nH]c(C(F)(F)F)c1Br", "O=c1cc[nH]c(-c2ncccc2)n1"),  # noqa
    ("CCOC(c1oc2ccccc2c1NC(CN1CCN(C)CC1)=O)=O", "O=C(CN1CCNCC1)Nc1coc2ccccc21"),  # noqa
    ("CSc1nsc(NN=Cc2ccc3c(c2)OCO3)c1C#N", "c1cc(sn1)NN=Cc1ccc2OCOc2c1"),  # noqa
    ("CC(C)(C)NC(NC(CSc1nc(C)c(C)c(C)n1)=O)=O", "c1cncnc1"),  # noqa
    ("Cc1cccnc1CN1CCN(Cc2onc(C(c3ccccc3)c3ccccc3)n2)CC1", "c1cccnc1CN1CCN(CC1)Cc1onc(n1)C(c1ccccc1)c1ccccc1"),  # noqa
    ("COc1ccc(Nc2oc3cc(=O)ccc-3cc2C(=O)Nc2ncccc2)cc1OC", "c1ccc(cc1)Nc1oc2-c(ccc(c2)=O)cc1C(Nc1ncccc1)=O"),  # noqa
    ("c1cc(C)c(OCC(NS(c2ccc(C)cc2)(=O)=O)=O)cc1", "O=C(COc1ccccc1)NS(=O)(=O)c1ccccc1"),  # noqa
    ("CCOc1ccc(-c2scc(CSc3sc(N)nn3)n2)cc1OC", "c1cccc(c1)-c1nc(cs1)CSc1scnn1"),  # noqa
    ("c1ccc(C(=O)COC(=O)CN2C(=O)C3C4CC(C3C2=O)C=C4)cc1", "c1ccc(C(=O)COC(=O)CN2C(=O)C3C4CC(C3C2=O)C=C4)cc1"),  # noqa
    ("Cc1occc1C(=O)NC(C)c1ccc2c(c1)OCO2", "O=C(NCc1ccc2c(c1)OCO2)c1ccoc1"),  # noqa
    ("CCn1c(SCC(=O)Nc2c(Cl)nccc2)nnc1-c1ccccc1", "O=C(Nc1cnccc1)CSc1[nH]c(nn1)-c1ccccc1"),  # noqa
    ("CCC(C)N(C)C1CCN(C(=S)Nc2cc(OC)ccc2)CC1", "S=C(Nc1ccccc1)N1CCCCC1"),  # noqa
    ("Brc1oc(C(=O)N2CC(=O)Nc3c(cc(Br)cc3)C2c2ccccc2)cc1", "O=C(N1CC(Nc2ccccc2C1c1ccccc1)=O)c1occc1"),  # noqa
    ("CN(C(=O)CCSc1nc(-c2cc3c(cc2)OCO3)cc(C(F)(F)F)n1)Cc1ccccc1", "O=C(NCc1ccccc1)CCSc1nc(ccn1)-c1cc2c(cc1)OCO2"),  # noqa
    ("[Br-].COc1c(OC)c(OC)cc(-c2nc3c[n+](CC(=O)c4ccccc4)ccc3n2C)c1", "O=C(C[n+]1cc2nc([nH]c2cc1)-c1ccccc1)c1ccccc1"),  # noqa
    ("CCOC(CSc1n(-c2c(OC)cccc2)c(CNC(Cc2ccccc2)=O)nn1)=O", "O=C(Cc1ccccc1)NCc1n(cnn1)-c1ccccc1"),  # noqa
    ("CS(N(Cc1ccccc1)c1ccc(C(Nc2c(Sc3ccccc3)cccc2)=O)cc1)(=O)=O", "O=C(c1ccc(NCc2ccccc2)cc1)Nc1c(cccc1)Sc1ccccc1"),  # noqa
    ("Cc1nc(C2N(C(=O)c3cn(C)c4c(c3=O)cccc4)CCc3c4c([nH]c32)cccc4)ccc1", "O=C(c1c[nH]c2c(cccc2)c1=O)N1C(c2ncccc2)c2[nH]c3ccccc3c2CC1"),  # noqa
    ("CCCCc1nc(N2CCOCC2)c(C#N)c2c1CCCC2", "c1nc(cc2c1CCCC2)N1CCOCC1"),  # noqa
    ("O=C(NN=Cc1cc([N+]([O-])=O)ccc1Cl)c1nccnc1", "O=C(NN=Cc1ccccc1)c1nccnc1"),  # noqa
    ("COc1ccc(-n2c(SCC(=O)c3ccc4c(c3)OCCO4)nnn2)cc1", "O=C(c1ccc2c(c1)OCCO2)CSc1n(nnn1)-c1ccccc1"),  # noqa
    ("COc1c(C=CC(=O)Nc2cc(S(NC3=NCCCCC3)(=O)=O)ccc2)cccc1", "O=C(Nc1cc(ccc1)S(=O)(=O)NC1=NCCCCC1)C=Cc1ccccc1"),  # noqa
    ("Cc1nn(-c2ccc(F)cc2)c(Cl)c1C=C(CC(=O)O)c1sc2ccccc2n1", "c1cc2sc(nc2cc1)C=Cc1cn(nc1)-c1ccccc1"),  # noqa
    ("COc1c(OC)c(OC)cc(C2N(c3ccccc3)OC3C2C(=O)N(Cc2ccccc2)C3=O)c1", "c1cccc(c1)CN1C(=O)C2C(N(OC2C1=O)c1ccccc1)c1ccccc1"),  # noqa
    ("COCCNC(=S)Nc1cc(OC)c(NC(=O)c2ccco2)cc1OC", "O=C(Nc1ccccc1)c1occc1"),  # noqa
    ("N#Cc1c(SCC(=O)c2cc3c(oc2=O)cccc3)nc(-c2ccccc2)cc1", "O=C(c1cc2c(cccc2)oc1=O)CSc1cccc(n1)-c1ccccc1"),  # noqa
    ("O=C(N1CCCC1)c1nc2ccccn2c1CN1CCCC(OCc2ccccc2)C1", "O=C(N1CCCC1)c1nc2ccccn2c1CN1CCCC(OCc2ccccc2)C1"),  # noqa
    ("Brc1cccc(OCCSc2ncccn2)c1", "c1cccc(c1)OCCSc1ncccn1"),  # noqa
    ("CC(C)(C)NC(=O)C12CCC(C)(C1(C)C)c1nc3ccccc3nc12", "c1cccc2nc3C4CC(CC4)c3nc12"),  # noqa
    ("[I-].CC(C)C1C(OCC(O)C[N+]2(C)CCCCC2)CC(C)CC1", "C1CC[NH+](CC1)CCCOC1CCCCC1"),  # noqa
    ("Cc1ccccc1NS(=O)(=O)c1ccc(OCC(=O)N2CCCCC2)cc1", "c1cc(ccc1)NS(=O)(=O)c1ccc(cc1)OCC(=O)N1CCCCC1"),  # noqa
    ("Cc1cc(NC(=O)CSc2nc3c(c(=O)n2-c2ccc(Br)cc2)SCC3)no1", "O=C(CSc1nc2c(c(n1-c1ccccc1)=O)SCC2)Nc1ccon1"),  # noqa
    ("Cc1ccccc1C(NC(C(C)C)C(OCC(c1[nH]ccc1)=O)=O)=O", "c1cc([nH]c1)C(COC(CNC(=O)c1ccccc1)=O)=O"),  # noqa
    ("Cc1ccnc(NS(c2ccc(NS(C)(=O)=O)cc2)(=O)=O)n1", "c1ccc(S(=O)(=O)Nc2ncccn2)cc1"),  # noqa
    ("Cn1c(-c2ccc(Cl)cc2)cnc1NCc1cc2c(cc1[N+]([O-])=O)OCO2.OC(=O)C(O)=O", "c1cc(ccc1)-c1[nH]c(nc1)NCc1cc2c(cc1)OCO2"),  # noqa
    ("CC1Cc2ccccc2N1C(=O)CON=Cc1ccc(OC(F)F)cc1", "O=C(CON=Cc1ccccc1)N1CCc2c1cccc2"),  # noqa
    ("C=C1C(=O)OC2C(O)C(C)=CC(=O)C=C(C)CC(OC(C(C)=CC)=O)C12", "C=C1C2CCC=CC(C=CCC2OC1=O)=O"),  # noqa
    ("O=C1C2N(CSC2)c2c(cc(C(F)(F)F)cc2)N1Cc1cccc(F)c1", "O=C1C2N(CSC2)c2ccccc2N1Cc1ccccc1"),  # noqa
    ("Cc1ccc(OCC(=O)Nc2c[nH]c(=O)[nH]c2=O)cc1C", "O=C(COc1ccccc1)Nc1c[nH]c([nH]c1=O)=O"),  # noqa
    ("Cn1c(CN2CCOCC2)nc2cc(NC(=O)c3ccccc3Cl)ccc12", "O=C(c1ccccc1)Nc1ccc2[nH]c(nc2c1)CN1CCOCC1"),  # noqa
    ("O=c1oc2ccc(O)cc2c(CN2CCN(CC=Cc3ccccc3)CC2)c1", "O=c1oc2ccccc2c(c1)CN1CCN(CC1)CC=Cc1ccccc1"),  # noqa
    ("Cn1c(Cc2ccccc2)nnc1SCCC(=O)Nc1ccccc1", "O=C(CCSc1nnc([nH]1)Cc1ccccc1)Nc1ccccc1"),  # noqa
    ("c1cc2nc(CC(=O)c3cc([N+]([O-])=O)ccc3)[nH]c2cc1", "O=C(Cc1nc2ccccc2[nH]1)c1ccccc1"),  # noqa
    ("c1cc2cc(C(=O)N3CCN(c4ccc(N5CCOCC5)nn4)CC3)c(=O)oc2cc1", "c1cc2cc(C(=O)N3CCN(c4ccc(N5CCOCC5)nn4)CC3)c(=O)oc2cc1"),  # noqa
    ("COc1ccccc1-n1c(=S)[nH]nc1CCn1nc(C)c(Br)c1C", "S=c1[nH]nc(n1-c1ccccc1)CCn1cccn1"),  # noqa
    ("CCC(=O)NC(=S)Nc1ccc(N2CCOCC2)cc1", "c1cccc(c1)N1CCOCC1"),  # noqa
    ("CCCCCC(=O)N1CCN(CCNC=C2C(=O)CC(c3ccc(OC)c(OC)c3)CC2=O)CC1", "c1ccc(cc1)C1CC(=O)C(C(=O)C1)=CNCCN1CCNCC1"),  # noqa
    ("CN1CCN(C(=O)CN(S(C)(=O)=O)Cc2ccc(Cl)cc2)CC1", "O=C(CNCc1ccccc1)N1CCNCC1"),  # noqa
    ("COc1cc(OC)cc(C(=O)NCc2cccnc2)c1", "O=C(NCc1cccnc1)c1ccccc1"),  # noqa
    ("c1cncc(NC(=O)C2CCCN(S(c3cccc4c3nsn4)(=O)=O)C2)c1", "c1cncc(NC(=O)C2CCCN(S(c3cccc4c3nsn4)(=O)=O)C2)c1"),  # noqa
    ("CC(NC1=NN(C(C)=O)C(C)(c2cccs2)S1)=O", "c1cc(sc1)C1SC=NN1"),  # noqa
    ("CCCC(=O)Nc1ccc(-c2nc3cc(C)c(C)cc3o2)cc1", "c1cccc(c1)-c1nc2ccccc2o1"),  # noqa
    ("Cc1c(C)n(CC(O)CN2CCOCC2)c2ccccc12.OC(=O)C(O)=O", "c1cn(c2ccccc12)CCCN1CCOCC1"),  # noqa
    ("Cc1occc1-c1n(CCc2ccccc2)c(SCC(=O)Nc2sccn2)nn1", "O=C(Nc1sccn1)CSc1n(c(nn1)-c1cocc1)CCc1ccccc1"),  # noqa
    ("Cc1oc(-c2cc(F)ccc2)nc1CN1C(CCc2ncccc2)CCCC1", "c1ccc(cc1)-c1nc(co1)CN1C(CCCC1)CCc1ncccc1"),  # noqa
    ("COc1c(OC)c(C(O)=O)c(C=NNC(c2cc(NC(c3ccc(F)cc3)=O)ccc2)=O)cc1", "O=C(Nc1cc(ccc1)C(=O)NN=Cc1ccccc1)c1ccccc1"),  # noqa
    ("CCn1c(Cc2ccccc2)nnc1SCC(=O)Nc1ccc(S(N)(=O)=O)cc1", "O=C(CSc1[nH]c(nn1)Cc1ccccc1)Nc1ccccc1"),  # noqa
    ("CCn1c(COc2nn(-c3ccccc3)c(=O)cc2)nnc1SCc1ccc(OC)cc1", "O=c1ccc(nn1-c1ccccc1)OCc1[nH]c(nn1)SCc1ccccc1"),  # noqa
    ("CC1=NC(=O)C(=C2CC(O)(C(F)(F)F)ON2)C(C)=C1", "O=C1C(=C2NOCC2)C=CC=N1"),  # noqa
    ("COc1ccc(NC(=S)Nc2ccccc2C(F)(F)F)cc1", "S=C(Nc1ccccc1)Nc1ccccc1"),  # noqa
    ("CCCc1cc(=O)nc(SCC(=O)c2cc(C)n(CCOC)c2C)[nH]1", "O=C(c1c[nH]cc1)CSc1[nH]ccc(=O)n1"),  # noqa
    ("CC(=O)Nc1ccc2c(c1)C(C)(C)C(C)N2C", "c1ccc2c(c1)NCC2"),  # noqa
    ("CCN1CCN(C(c2ccc(OCC(Nc3ccc(F)cc3)=O)c(OC)c2)=O)CC1", "c1cc(ccc1)NC(=O)COc1ccc(C(N2CCNCC2)=O)cc1"),  # noqa
    ("CCCCN1C2CCCC1CC(NC(=O)c1ccc(OC)c(OC)c1)C2", "O=C(NC1CC2NC(CCC2)C1)c1ccccc1"),  # noqa
    ("c1ccc(N(CC(=O)N2CCOCC2)S(c2ccccc2)(=O)=O)cc1", "c1ccc(N(CC(=O)N2CCOCC2)S(c2ccccc2)(=O)=O)cc1"),  # noqa
    ("CCn1c(C)nc2cc(C(=O)NN=Cc3ccc(OC)c(O)c3)ccc12", "O=C(NN=Cc1ccccc1)c1ccc2[nH]cnc2c1"),  # noqa
    ("[Cl-].NC(=O)CN1C=CC(=C[NH+]=O)C=C1", "C=C1C=CNC=C1"),  # noqa
    ("Cn1cnnc1SC1C(NS(c2ccccc2)(=O)=O)c2c3c(ccc2)cccc31", "O=S(=O)(NC1C(Sc2[nH]cnn2)c2cccc3c2c1ccc3)c1ccccc1"),  # noqa
    ("COc1ccc(Nc2nc(NCc3ccco3)nc(NN=Cc3ccccc3F)n2)cc1", "c1ccc(Nc2nc(nc(n2)NN=Cc2ccccc2)NCc2ccco2)cc1"),  # noqa
    ("CC1=CC(=O)C(=C2C=C(c3ccccc3[N+]([O-])=O)NN2)C=C1", "O=C1C(=C2NNC(=C2)c2ccccc2)C=CC=C1"),  # noqa
    ("COc1ccc(CC2[N+]([O-])(C)CCc3cc(OC)c(O)cc32)cc1O", "c1ccc(cc1)CC1c2c(cccc2)CC[NH2+]1"),  # noqa
    ("Cl.NC(N)=Nc1nc(=O)c2cc(Br)ccc2[nH]1", "O=c1nc[nH]c2ccccc21"),  # noqa
    ("CC(=O)N1CCC(=NNc2ccc(S(=O)(=O)N3CCOCC3)cc2[N+]([O-])=O)CC1", "c1cc(ccc1NN=C1CCNCC1)S(=O)(=O)N1CCOCC1"),  # noqa
    ("Cc1cc(S(N(Cc2ccc(F)cc2)CC2OCCC2)(=O)=O)ccc1-n1cnnn1", "c1cc(ccc1)CN(CC1OCCC1)S(c1ccc(cc1)-n1cnnn1)(=O)=O"),  # noqa
    ("CC1(C)OCc2c(c3c(sc4c(NCCCO)ncnc43)nc2-c2ccco2)C1", "c1ncnc2c1sc1nc(c3c(c12)CCOC3)-c1ccco1"),  # noqa
    ("COc1ccc(CCNC(=O)CSc2n(-c3ccc(OC)c(OC)c3)nnn2)cc1OC", "O=C(CSc1n(-c2ccccc2)nnn1)NCCc1ccccc1"),  # noqa
    ("CC(C)(CC(O)=O)CC(NCc1c(Cl)cccc1Sc1ccc(Cl)cc1)=O", "c1ccc(Sc2ccccc2)cc1"),  # noqa
    ("COc1ccc(-c2cc(CCCC(=O)NCCc3cc(OC)ccc3OC)no2)cc1", "O=C(NCCc1ccccc1)CCCc1noc(c1)-c1ccccc1"),  # noqa
    ("Cc1ccc(-c2ncns2)cc1", "c1ccc(cc1)-c1sncn1"),  # noqa
    ("C(O)CCn1c(=O)c2c(nc1C=Cc1ccc([N+]([O-])=O)o1)cccc2", "O=c1[nH]c(C=Cc2ccco2)nc2c1cccc2"),  # noqa
    ("COC(CC(O)CC(O)C(C)OCc1ccccc1)OC", "c1ccccc1"),  # noqa
    ("Cl.CCCC(N1CCN(C(=O)c2occc2)CC1)c1n(C(C)(C)C)nnn1", "O=C(N1CCN(Cc2nnn[nH]2)CC1)c1ccco1"),  # noqa
    ("O=C(NC(CO)c1ccccc1)c1occc1", "O=C(NCc1ccccc1)c1occc1"),  # noqa
    ("O=C(Nc1ccc(N2CCOCC2)cc1)c1c(Cl)cc(F)c(F)c1", "O=C(Nc1ccc(N2CCOCC2)cc1)c1ccccc1"),  # noqa
    ("CCc1sc(N2C(=O)c3ccc(Oc4ccc([N+]([O-])=O)cc4)cc3C2=O)nn1", "O=C1N(C(=O)c2cc(Oc3ccccc3)ccc21)c1scnn1"),  # noqa
    ("CC(C)Cc1ccc(C(C)C(=O)O)cc1", "c1ccccc1"),  # noqa
    ("Cl.N=c1sccn1CC(=O)Nc1cc(S(N2CCCC2)(=O)=O)ccc1Cl", "N=c1n(CC(=O)Nc2cccc(S(=O)(N3CCCC3)=O)c2)ccs1"),  # noqa
    ("c1ccc(-c2ccc(C(=O)OC3CC4OC(=O)CC4C3CO)cc2)cc1", "c1ccc(cc1)-c1ccc(C(=O)OC2CC3CC(=O)OC3C2)cc1"),  # noqa
    ("CN(CCC#N)CC(=O)Nc1ccc(S(N)(=O)=O)cc1", "c1ccccc1"),  # noqa
    ("Cc1nc(-c2ccc([N+]([O-])=O)cc2)sc1C(=O)O", "c1cc(-c2sccn2)ccc1"),  # noqa
    ("c1coc(C(=O)N2CCN(C(Cn3nnc(-c4ccc(NC(c5ccc(F)cc5)=O)cc4)n3)=O)CC2)c1", "O=C(N1CCN(C(=O)Cn2nc(nn2)-c2ccc(NC(=O)c3ccccc3)cc2)CC1)c1ccco1"),  # noqa
    ("Cc1onc(-c2c(Cl)cccc2Cl)c1C(N)=S", "c1ccc(cc1)-c1nocc1"),  # noqa
    ("CCOC(=O)c1cnc2ccccc2c1NCCO", "c1cnc2ccccc2c1"),  # noqa
    ("Cc1ccc(C)c(NC(=O)Cn2nnc(-c3ccc(N4CCOCC4)cc3)n2)c1", "O=C(Cn1nnc(n1)-c1ccc(cc1)N1CCOCC1)Nc1ccccc1"),  # noqa
    ("CC(C)(C)c1cc(C(=O)NNc2ccc(OC(F)(F)F)cc2)n(Cc2ccccc2)n1", "O=C(NNc1ccccc1)c1ccnn1Cc1ccccc1"),  # noqa
    ("CCCCCOC(=O)C1=C(C)N=C2N(NN=N2)C1c1ccc(OC)c(OC)c1OC", "c1cccc(c1)C1N2NN=NC2=NC=C1"),  # noqa
    ("Cc1cc2cc(CNC(=O)C3CC3)ccc2n1C", "O=C(NCc1ccc2c(cc[nH]2)c1)C1CC1"),  # noqa
    ("Cc1ccccc1C(NC(CC(C)C)C(Nc1cc(S(N(C)C)(=O)=O)ccc1)=O)=O", "c1ccc(cc1)NC(CNC(=O)c1ccccc1)=O"),  # noqa
    ("COCCCNC(=S)N1CCC(NC(=O)c2ccco2)CC1", "O=C(NC1CCNCC1)c1ccco1"),  # noqa
    ("Cn1c(C=Cc2oc([N+]([O-])=O)cc2)nc2ccccc2c1=O", "O=c1[nH]c(C=Cc2occc2)nc2ccccc12"),  # noqa
    ("c1cc2nc(SCc3cc(=O)n4ccsc4n3)n(CCCO)c(=O)c2cc1", "c1ccc2nc(SCc3cc(=O)n4ccsc4n3)[nH]c(=O)c2c1"),  # noqa
    ("c1ccc2c(c1)cccc2NC(=O)CC1SC(NCC2OCCC2)=NC1=O", "c1ccc2c(c1)cccc2NC(=O)CC1SC(NCC2OCCC2)=NC1=O"),  # noqa
  ]

  def test1MurckoScaffold(self):
    for testMol in self.testMolecules:
      mol = Chem.MolFromSmiles(testMol[0])
      calcScaffold = Chem.MolToSmiles(GetScaffoldForMol(mol))
      actualScaffold = Chem.MolToSmiles(Chem.MolFromSmiles(testMol[1]))
      self.assertEqual(calcScaffold, actualScaffold)
  def test2MurckoScaffold(self):
    for testMol in self.testMolecules2:
      mol = Chem.MolFromSmiles(testMol[0])
      calcScaffold = Chem.MolToSmiles(GetScaffoldForMol(mol))
      actualScaffold = Chem.MolToSmiles(Chem.MolFromSmiles(testMol[1]))
      self.assertEqual(calcScaffold, actualScaffold)


if __name__ == '__main__':  # pragma: no cover
  unittest.main()

