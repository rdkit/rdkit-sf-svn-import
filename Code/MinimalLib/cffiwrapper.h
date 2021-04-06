//
//  Copyright (C) 2021 Greg Landrum
//
//   @@ All Rights Reserved @@
//  This file is part of the RDKit.
//  The contents are covered by the terms of the BSD license
//  which is included in the file license.txt, found at the root
//  of the RDKit source tree.
//
#pragma once

#include <RDGeneral/export.h>
#ifdef RDKIT_RDKITCFFI_BUILD
#define RDKIT_RDKITCFFI_EXPORT RDKIT_EXPORT_API
#else
#define RDKIT_RDKITCFFI_EXPORT RDKIT_IMPORT_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

// I/O
RDKIT_RDKITCFFI_EXPORT char *get_mol(const char *input, size_t *mol_sz);
RDKIT_RDKITCFFI_EXPORT char *get_qmol(const char *input, size_t *mol_sz);
RDKIT_RDKITCFFI_EXPORT char *get_molblock(const char *pkl, size_t pkl_sz);
RDKIT_RDKITCFFI_EXPORT char *get_v3kmolblock(const char *pkl, size_t pkl_sz);
RDKIT_RDKITCFFI_EXPORT char *get_smiles(const char *pkl, size_t pkl_sz);
RDKIT_RDKITCFFI_EXPORT char *get_smarts(const char *pkl, size_t pkl_sz);
RDKIT_RDKITCFFI_EXPORT char *get_cxsmiles(const char *pkl, size_t pkl_sz);
RDKIT_RDKITCFFI_EXPORT char *get_json(const char *pkl, size_t pkl_sz);
RDKIT_RDKITCFFI_EXPORT char *get_inchi(const char *pkl, size_t pkl_sz);
RDKIT_RDKITCFFI_EXPORT char *get_inchi_for_molblock(const char *ctab);
RDKIT_RDKITCFFI_EXPORT char *get_inchikey_for_inchi(const char *inchi);

// substructure
RDKIT_RDKITCFFI_EXPORT char *get_substruct_match(const char *mol_pkl,
                                                 size_t mol_pkl_sz,
                                                 const char *query_pkl,
                                                 size_t query_pkl_sz,
                                                 const char *options_json);
RDKIT_RDKITCFFI_EXPORT char *get_substruct_matches(const char *mol_pkl,
                                                   size_t mol_pkl_sz,
                                                   const char *query_pkl,
                                                   size_t query_pkl_sz,
                                                   const char *options_json);

// Drawing
RDKIT_RDKITCFFI_EXPORT char *get_svg(const char *pkl, size_t pkl_sz,
                                     const char *details_json);

// Calculators
RDKIT_RDKITCFFI_EXPORT char *get_descriptors(const char *pkl, size_t pkl_sz);
RDKIT_RDKITCFFI_EXPORT char *get_morgan_fp(const char *pkl, size_t pkl_sz,
                                           const char *details_json);
RDKIT_RDKITCFFI_EXPORT char *get_morgan_fp_as_bytes(const char *pkl,
                                                    size_t pkl_sz,
                                                    size_t *nbytes,
                                                    const char *details_json);
RDKIT_RDKITCFFI_EXPORT char *get_rdkit_fp(const char *pkl, size_t pkl_sz,
                                          const char *details_json);
RDKIT_RDKITCFFI_EXPORT char *get_rdkit_fp_as_bytes(const char *pkl,
                                                   size_t pkl_sz,
                                                   size_t *nbytes,
                                                   const char *details_json);
RDKIT_RDKITCFFI_EXPORT char *get_pattern_fp(const char *pkl, size_t pkl_sz,
                                            const char *details_json);
RDKIT_RDKITCFFI_EXPORT char *get_pattern_fp_as_bytes(const char *pkl,
                                                     size_t pkl_sz,
                                                     size_t *nbytes,
                                                     const char *details_json);

// modification
RDKIT_RDKITCFFI_EXPORT short add_hs(char **pkl, size_t *pkl_sz);
RDKIT_RDKITCFFI_EXPORT short remove_hs(char **pkl, size_t *pkl_sz);

// coordinates
RDKIT_RDKITCFFI_EXPORT void prefer_coordgen(short val);
RDKIT_RDKITCFFI_EXPORT short set_2d_coords(char **pkl, size_t *pkl_sz);
RDKIT_RDKITCFFI_EXPORT short set_3d_coords(char **pkl, size_t *pkl_sz,
                                           const char *params_json);
RDKIT_RDKITCFFI_EXPORT short set_2d_coords_aligned(char **pkl, size_t *pkl_sz,
                                                   const char *template_pkl,
                                                   size_t template_sz,
                                                   const char *details_json);

// housekeeping
RDKIT_RDKITCFFI_EXPORT void free_ptr(char *ptr);

RDKIT_RDKITCFFI_EXPORT char *version();
RDKIT_RDKITCFFI_EXPORT void enable_logging();
RDKIT_RDKITCFFI_EXPORT void disable_logging();

#ifdef __cplusplus
}
#endif
