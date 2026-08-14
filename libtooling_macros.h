#pragma once

#define TRY_TO_TRAVERSE_OR_ENQUEUE_STMT(S)                                     \
  do {                                                                         \
    if (!TRAVERSE_STMT_BASE(Stmt, Stmt, S, Queue))                             \
      return false;                                                            \
  } while (false)

#define DEF_TRAVERSE_TMPL_DECL(TMPLDECLKIND)                                   \
  DEF_TRAVERSE_DECL(TMPLDECLKIND##TemplateDecl, {                              \
    TRY_TO(TraverseTemplateParameterListHelper(D->getTemplateParameters()));   \
    TRY_TO(TraverseDecl(D->getTemplatedDecl()));                               \
                                                                               \
    /* By default, we do not traverse the instantiations of                    \
       class templates since they do not appear in the user code. The          \
       following code optionally traverses them.                               \
                                                                               \
       We only traverse the class instantiations when we see the canonical     \
       declaration of the template, to ensure we only visit them once. */      \
    if (getDerived().shouldVisitTemplateInstantiations() &&                    \
        D == D->getCanonicalDecl())                                            \
      TRY_TO(TraverseTemplateInstantiations(D));                               \
                                                                               \
    /* Note that getInstantiatedFromMemberTemplate() is just a link            \
       from a template instantiation back to the template from which           \
       it was instantiated, and thus should not be traversed. */               \
  })

#define DEF_TRAVERSE_TMPL_SPEC_DECL(TMPLDECLKIND, DECLKIND)                    \
  DEF_TRAVERSE_DECL(TMPLDECLKIND##TemplateSpecializationDecl, {                \
    /* For implicit instantiations ("set<int> x;"), we don't want to           \
       recurse at all, since the instatiated template isn't written in         \
       the source code anywhere.  (Note the instatiated *type* --              \
       set<int> -- is written, and will still get a callback of                \
       TemplateSpecializationType).  For explicit instantiations               \
       ("template set<int>;"), we do need a callback, since this               \
       is the only callback that's made for this instantiation.                \
       We use getTemplateArgsAsWritten() to distinguish. */                    \
    if (const auto *ArgsWritten = D->getTemplateArgsAsWritten()) {             \
      assert(D->getTemplateSpecializationKind() != TSK_ImplicitInstantiation); \
      /* The args that remains unspecialized. */                               \
      TRY_TO(TraverseTemplateArgumentLocsHelper(                               \
          ArgsWritten->getTemplateArgs(), ArgsWritten->NumTemplateArgs));      \
    }                                                                          \
                                                                               \
    if (getDerived().shouldVisitTemplateInstantiations() ||                    \
        D->getTemplateSpecializationKind() == TSK_ExplicitSpecialization) {    \
      /* Traverse base definition for explicit specializations */              \
      TRY_TO(Traverse##DECLKIND##Helper(D));                                   \
    } else {                                                                   \
      TRY_TO(TraverseNestedNameSpecifierLoc(D->getQualifierLoc()));            \
                                                                               \
      /* Returning from here skips traversing the                              \
         declaration context of the *TemplateSpecializationDecl                \
         (embedded in the DEF_TRAVERSE_DECL() macro)                           \
         which contains the instantiated members of the template. */           \
      return true;                                                             \
    }                                                                          \
  })

#define DEF_TRAVERSE_TMPL_PART_SPEC_DECL(TMPLDECLKIND, DECLKIND)               \
  DEF_TRAVERSE_DECL(TMPLDECLKIND##TemplatePartialSpecializationDecl, {         \
    /* The partial specialization. */                                          \
    TRY_TO(TraverseTemplateParameterListHelper(D->getTemplateParameters()));   \
    /* The args that remains unspecialized. */                                 \
    TRY_TO(TraverseTemplateArgumentLocsHelper(                                 \
        D->getTemplateArgsAsWritten()->getTemplateArgs(),                      \
        D->getTemplateArgsAsWritten()->NumTemplateArgs));                      \
                                                                               \
    /* Don't need the *TemplatePartialSpecializationHelper, even               \
       though that's our parent class -- we already visit all the              \
       template args here. */                                                  \
    TRY_TO(Traverse##DECLKIND##Helper(D));                                     \
                                                                               \
    /* Instantiations will have been visited with the primary template. */     \
  })

#define NEVER_CANONICAL_TYPE(Class)                                            \
  template <> inline const Class##Type *Type::getAsCanonical() const = delete; \
  template <> inline const Class##Type *Type::castAsCanonical() const = delete;

#define clEnumVal(ENUMVAL, DESC)                                               \
  llvm::cl::OptionEnumValue { #ENUMVAL, int(ENUMVAL), DESC }

#define clEnumValN(ENUMVAL, FLAGNAME, DESC)                                    \
  llvm::cl::OptionEnumValue { FLAGNAME, int(ENUMVAL), DESC }

