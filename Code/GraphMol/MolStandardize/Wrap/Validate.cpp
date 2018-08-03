#include <RDBoost/Wrap.h>

#include <GraphMol/RDKitBase.h>
#include <GraphMol/MolStandardize/Validate.h>

namespace python = boost::python;
using namespace RDKit;

namespace {

python::list rdkitValidate(MolStandardize::RDKitValidation &self,
                           const ROMol &mol, const bool reportAllFailures) {
  python::list res;
  std::vector<MolStandardize::ValidationErrorInfo> errout =
      self.validate(mol, reportAllFailures);
  for (auto &query : errout) {
    std::string msg = query.message();
    res.append(msg);
  }
  return res;
}

MolStandardize::MolVSValidation *getMolVSValidation(
    python::object validations) {
  std::vector<MolStandardize::MolVSValidations *> vs;

  std::unique_ptr<std::vector<MolStandardize::MolVSValidations *>> pvect =
      pythonObjectToVect<MolStandardize::MolVSValidations *>(validations);

  for (auto v : *pvect) {
    vs.push_back(v->copy());
  }
  return new MolStandardize::MolVSValidation(vs);
}

python::list molVSvalidateHelper(MolStandardize::MolVSValidation &self,
	const ROMol &mol, bool reportAllFailures) {
		python::list s;
		std::vector<MolStandardize::ValidationErrorInfo> errout = 
						self.validate(mol, reportAllFailures);
		for (auto &query : errout) {
			s.append(query.message());
		}
		return s;
}

MolStandardize::AllowedAtomsValidation *getAllowedAtomsValidation(
    python::object atoms) {
//  std::vector< std::shared_ptr<Atom> atomList;

//  std::unique_ptr<std::vector< std::shared_ptr<Atom> >> p_atomList =
//     pythonObjectToVect< std::shared_ptr<Atom> >(atoms);
    std::unique_ptr<std::vector<Atom *> > p_atomList = 
						pythonObjectToVect<Atom *>(atoms);     
		std::vector<std::shared_ptr<Atom>> satoms;
	  for (auto ap : *p_atomList) {
 			satoms.push_back(std::shared_ptr<Atom>(ap->copy()));
		}

  return new MolStandardize::AllowedAtomsValidation(satoms);
}


python::list allowedAtomsValidate(MolStandardize::AllowedAtomsValidation &self,
                           const ROMol &mol, const bool reportAllFailures) {
  python::list res;
  std::vector<MolStandardize::ValidationErrorInfo> errout =
      self.validate(mol, reportAllFailures);
  for (auto &query : errout) {
    std::string msg = query.message();
    res.append(msg);
  }
  return res;
}

//python::list rdkitValidate(MolStandardize::RDKitValidation &self,
//                           const ROMol &mol, const bool reportAllFailures) {
//  python::list res;
//  std::vector<MolStandardize::ValidationErrorInfo> errout =
//      self.validate(mol, reportAllFailures);
//  for (auto &query : errout) {
//    std::string msg = query.message();
//    res.append(msg);
//  }
//  return res;
//}

}  // namespace
struct validate_wrapper {
  static void wrap() {
    std::string docString = "";

    python::class_<MolStandardize::RDKitValidation, boost::noncopyable>(
        "RDKitValidation", python::init<>())
        .def("validate", rdkitValidate,
             (python::arg("self"), python::arg("mol"),
              python::arg("reportAllFailures") = false),
             "");

    python::class_<MolStandardize::MolVSValidations, boost::noncopyable>(
        "MolVSValidations", python::no_init)
        .def("run", &MolStandardize::MolVSValidations::run,
             (python::arg("self"), python::arg("mol"),
              python::arg("reportAllFailures"), python::arg("errors")),
             "");

    python::class_<MolStandardize::NoAtomValidation,
                   python::bases<MolStandardize::MolVSValidations>>(
        "NoAtomValidation", python::init<>())
        .def("run", &MolStandardize::NoAtomValidation::run,
             (python::arg("self"), python::arg("mol"),
              python::arg("reportAllFailures"), python::arg("errors")),
             "");

    python::class_<MolStandardize::FragmentValidation,
                   python::bases<MolStandardize::MolVSValidations>>(
        "FragmentValidation", python::init<>())
        .def("run", &MolStandardize::FragmentValidation::run,
             (python::arg("self"), python::arg("mol"),
              python::arg("reportAllFailures"), python::arg("errors")),
             "");
    python::class_<MolStandardize::NeutralValidation,
                   python::bases<MolStandardize::MolVSValidations>>(
        "NeutralValidation", python::init<>())
        .def("run", &MolStandardize::NeutralValidation::run,
             (python::arg("self"), python::arg("mol"),
              python::arg("reportAllFailures"), python::arg("errors")),
             "");
    python::class_<MolStandardize::IsotopeValidation,
                   python::bases<MolStandardize::MolVSValidations>>(
        "IsotopeValidation", python::init<>())
        .def("run", &MolStandardize::IsotopeValidation::run,
             (python::arg("self"), python::arg("mol"),
              python::arg("reportAllFailures"), python::arg("errors")),
             "");

    python::class_<MolStandardize::MolVSValidation, boost::noncopyable>(
        "MolVSValidation")
        .def("__init__", python::make_constructor(&getMolVSValidation))
        .def("validate", molVSvalidateHelper,
             (python::arg("self"), python::arg("mol"),
              python::arg("reportAllFailures") = false),
             "");

    python::class_<MolStandardize::AllowedAtomsValidation, boost::noncopyable>(
        "AllowedAtomsValidation", python::init<>())
				.def("__init__", python::make_constructor(&getAllowedAtomsValidation))
//        .def("validate", allowedAtomsValidate,
//             (python::arg("self"), python::arg("mol"),
//              python::arg("reportAllFailures") = false),
//             "");
             ;

//    python::class_<MolStandardize::RDKitValidation, boost::noncopyable>(
//        "RDKitValidation", python::init<>())
//        .def("validate", rdkitValidate,
//             (python::arg("self"), python::arg("mol"),
//              python::arg("reportAllFailures") = false),
//             "");

  }
};

void wrap_validate() { validate_wrapper::wrap(); }
