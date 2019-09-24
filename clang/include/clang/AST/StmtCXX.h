//===--- StmtCXX.h - Classes for representing C++ statements ----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines the C++ statement AST node classes.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_AST_STMTCXX_H
#define LLVM_CLANG_AST_STMTCXX_H

#include "clang/AST/DeclarationName.h"
#include "clang/AST/Expr.h"
#include "clang/AST/NestedNameSpecifier.h"
#include "clang/AST/Stmt.h"
#include "llvm/Support/Compiler.h"

namespace clang {

class VarDecl;

/// CXXCatchStmt - This represents a C++ catch block.
///
class CXXCatchStmt : public Stmt {
  SourceLocation CatchLoc;
  /// The exception-declaration of the type.
  VarDecl *ExceptionDecl;
  /// The handler block.
  Stmt *HandlerBlock;

public:
  CXXCatchStmt(SourceLocation catchLoc, VarDecl *exDecl, Stmt *handlerBlock)
  : Stmt(CXXCatchStmtClass), CatchLoc(catchLoc), ExceptionDecl(exDecl),
    HandlerBlock(handlerBlock) {}

  CXXCatchStmt(EmptyShell Empty)
  : Stmt(CXXCatchStmtClass), ExceptionDecl(nullptr), HandlerBlock(nullptr) {}

  SourceLocation getBeginLoc() const LLVM_READONLY { return CatchLoc; }
  SourceLocation getEndLoc() const LLVM_READONLY {
    return HandlerBlock->getEndLoc();
  }

  SourceLocation getCatchLoc() const { return CatchLoc; }
  VarDecl *getExceptionDecl() const { return ExceptionDecl; }
  QualType getCaughtType() const;
  Stmt *getHandlerBlock() const { return HandlerBlock; }

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == CXXCatchStmtClass;
  }

  child_range children() { return child_range(&HandlerBlock, &HandlerBlock+1); }

  const_child_range children() const {
    return const_child_range(&HandlerBlock, &HandlerBlock + 1);
  }

  friend class ASTStmtReader;
};

/// CXXTryStmt - A C++ try block, including all handlers.
///
class CXXTryStmt final : public Stmt,
                         private llvm::TrailingObjects<CXXTryStmt, Stmt *> {

  friend TrailingObjects;
  friend class ASTStmtReader;

  SourceLocation TryLoc;
  unsigned NumHandlers;
  size_t numTrailingObjects(OverloadToken<Stmt *>) const { return NumHandlers; }

  CXXTryStmt(SourceLocation tryLoc, Stmt *tryBlock, ArrayRef<Stmt*> handlers);
  CXXTryStmt(EmptyShell Empty, unsigned numHandlers)
    : Stmt(CXXTryStmtClass), NumHandlers(numHandlers) { }

  Stmt *const *getStmts() const { return getTrailingObjects<Stmt *>(); }
  Stmt **getStmts() { return getTrailingObjects<Stmt *>(); }

public:
  static CXXTryStmt *Create(const ASTContext &C, SourceLocation tryLoc,
                            Stmt *tryBlock, ArrayRef<Stmt*> handlers);

  static CXXTryStmt *Create(const ASTContext &C, EmptyShell Empty,
                            unsigned numHandlers);

  SourceLocation getBeginLoc() const LLVM_READONLY { return getTryLoc(); }

  SourceLocation getTryLoc() const { return TryLoc; }
  SourceLocation getEndLoc() const {
    return getStmts()[NumHandlers]->getEndLoc();
  }

  CompoundStmt *getTryBlock() {
    return cast<CompoundStmt>(getStmts()[0]);
  }
  const CompoundStmt *getTryBlock() const {
    return cast<CompoundStmt>(getStmts()[0]);
  }

  unsigned getNumHandlers() const { return NumHandlers; }
  CXXCatchStmt *getHandler(unsigned i) {
    return cast<CXXCatchStmt>(getStmts()[i + 1]);
  }
  const CXXCatchStmt *getHandler(unsigned i) const {
    return cast<CXXCatchStmt>(getStmts()[i + 1]);
  }

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == CXXTryStmtClass;
  }

  child_range children() {
    return child_range(getStmts(), getStmts() + getNumHandlers() + 1);
  }

  const_child_range children() const {
    return const_child_range(getStmts(), getStmts() + getNumHandlers() + 1);
  }
};

/// CXXForRangeStmt - This represents C++0x [stmt.ranged]'s ranged for
/// statement, represented as 'for (range-declarator : range-expression)'
/// or 'for (init-statement range-declarator : range-expression)'.
///
/// This is stored in a partially-desugared form to allow full semantic
/// analysis of the constituent components. The original syntactic components
/// can be extracted using getLoopVariable and getRangeInit.
class CXXForRangeStmt : public Stmt {
  SourceLocation ForLoc;
  enum { INIT, RANGE, BEGINSTMT, ENDSTMT, COND, INC, LOOPVAR, BODY, END };
  // SubExprs[RANGE] is an expression or declstmt.
  // SubExprs[COND] and SubExprs[INC] are expressions.
  Stmt *SubExprs[END];
  SourceLocation CoawaitLoc;
  SourceLocation ColonLoc;
  SourceLocation RParenLoc;

  friend class ASTStmtReader;
public:
  CXXForRangeStmt(Stmt *InitStmt, DeclStmt *Range, DeclStmt *Begin,
                  DeclStmt *End, Expr *Cond, Expr *Inc, DeclStmt *LoopVar,
                  Stmt *Body, SourceLocation FL, SourceLocation CAL,
                  SourceLocation CL, SourceLocation RPL);
  CXXForRangeStmt(EmptyShell Empty) : Stmt(CXXForRangeStmtClass, Empty) { }

  Stmt *getInit() { return SubExprs[INIT]; }
  VarDecl *getLoopVariable();
  Expr *getRangeInit();

  const Stmt *getInit() const { return SubExprs[INIT]; }
  const VarDecl *getLoopVariable() const;
  const Expr *getRangeInit() const;


  DeclStmt *getRangeStmt() { return cast<DeclStmt>(SubExprs[RANGE]); }
  DeclStmt *getBeginStmt() {
    return cast_or_null<DeclStmt>(SubExprs[BEGINSTMT]);
  }
  DeclStmt *getEndStmt() { return cast_or_null<DeclStmt>(SubExprs[ENDSTMT]); }
  Expr *getCond() { return cast_or_null<Expr>(SubExprs[COND]); }
  Expr *getInc() { return cast_or_null<Expr>(SubExprs[INC]); }
  DeclStmt *getLoopVarStmt() { return cast<DeclStmt>(SubExprs[LOOPVAR]); }
  Stmt *getBody() { return SubExprs[BODY]; }

  const DeclStmt *getRangeStmt() const {
    return cast<DeclStmt>(SubExprs[RANGE]);
  }
  const DeclStmt *getBeginStmt() const {
    return cast_or_null<DeclStmt>(SubExprs[BEGINSTMT]);
  }
  const DeclStmt *getEndStmt() const {
    return cast_or_null<DeclStmt>(SubExprs[ENDSTMT]);
  }
  const Expr *getCond() const {
    return cast_or_null<Expr>(SubExprs[COND]);
  }
  const Expr *getInc() const {
    return cast_or_null<Expr>(SubExprs[INC]);
  }
  const DeclStmt *getLoopVarStmt() const {
    return cast<DeclStmt>(SubExprs[LOOPVAR]);
  }
  const Stmt *getBody() const { return SubExprs[BODY]; }

  void setInit(Stmt *S) { SubExprs[INIT] = S; }
  void setRangeInit(Expr *E) { SubExprs[RANGE] = reinterpret_cast<Stmt*>(E); }
  void setRangeStmt(Stmt *S) { SubExprs[RANGE] = S; }
  void setBeginStmt(Stmt *S) { SubExprs[BEGINSTMT] = S; }
  void setEndStmt(Stmt *S) { SubExprs[ENDSTMT] = S; }
  void setCond(Expr *E) { SubExprs[COND] = reinterpret_cast<Stmt*>(E); }
  void setInc(Expr *E) { SubExprs[INC] = reinterpret_cast<Stmt*>(E); }
  void setLoopVarStmt(Stmt *S) { SubExprs[LOOPVAR] = S; }
  void setBody(Stmt *S) { SubExprs[BODY] = S; }

  SourceLocation getForLoc() const { return ForLoc; }
  SourceLocation getCoawaitLoc() const { return CoawaitLoc; }
  SourceLocation getColonLoc() const { return ColonLoc; }
  SourceLocation getRParenLoc() const { return RParenLoc; }

  SourceLocation getBeginLoc() const LLVM_READONLY { return ForLoc; }
  SourceLocation getEndLoc() const LLVM_READONLY {
    return SubExprs[BODY]->getEndLoc();
  }

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == CXXForRangeStmtClass;
  }

  // Iterators
  child_range children() {
    return child_range(&SubExprs[0], &SubExprs[END]);
  }

  const_child_range children() const {
    return const_child_range(&SubExprs[0], &SubExprs[END]);
  }
};

/// Representation of a Microsoft __if_exists or __if_not_exists
/// statement with a dependent name.
///
/// The __if_exists statement can be used to include a sequence of statements
/// in the program only when a particular dependent name does not exist. For
/// example:
///
/// \code
/// template<typename T>
/// void call_foo(T &t) {
///   __if_exists (T::foo) {
///     t.foo(); // okay: only called when T::foo exists.
///   }
/// }
/// \endcode
///
/// Similarly, the __if_not_exists statement can be used to include the
/// statements when a particular name does not exist.
///
/// Note that this statement only captures __if_exists and __if_not_exists
/// statements whose name is dependent. All non-dependent cases are handled
/// directly in the parser, so that they don't introduce a new scope. Clang
/// introduces scopes in the dependent case to keep names inside the compound
/// statement from leaking out into the surround statements, which would
/// compromise the template instantiation model. This behavior differs from
/// Visual C++ (which never introduces a scope), but is a fairly reasonable
/// approximation of the VC++ behavior.
class MSDependentExistsStmt : public Stmt {
  SourceLocation KeywordLoc;
  bool IsIfExists;
  NestedNameSpecifierLoc QualifierLoc;
  DeclarationNameInfo NameInfo;
  Stmt *SubStmt;

  friend class ASTReader;
  friend class ASTStmtReader;

public:
  MSDependentExistsStmt(SourceLocation KeywordLoc, bool IsIfExists,
                        NestedNameSpecifierLoc QualifierLoc,
                        DeclarationNameInfo NameInfo,
                        CompoundStmt *SubStmt)
  : Stmt(MSDependentExistsStmtClass),
    KeywordLoc(KeywordLoc), IsIfExists(IsIfExists),
    QualifierLoc(QualifierLoc), NameInfo(NameInfo),
    SubStmt(reinterpret_cast<Stmt *>(SubStmt)) { }

  /// Retrieve the location of the __if_exists or __if_not_exists
  /// keyword.
  SourceLocation getKeywordLoc() const { return KeywordLoc; }

  /// Determine whether this is an __if_exists statement.
  bool isIfExists() const { return IsIfExists; }

  /// Determine whether this is an __if_exists statement.
  bool isIfNotExists() const { return !IsIfExists; }

  /// Retrieve the nested-name-specifier that qualifies this name, if
  /// any.
  NestedNameSpecifierLoc getQualifierLoc() const { return QualifierLoc; }

  /// Retrieve the name of the entity we're testing for, along with
  /// location information
  DeclarationNameInfo getNameInfo() const { return NameInfo; }

  /// Retrieve the compound statement that will be included in the
  /// program only if the existence of the symbol matches the initial keyword.
  CompoundStmt *getSubStmt() const {
    return reinterpret_cast<CompoundStmt *>(SubStmt);
  }

  SourceLocation getBeginLoc() const LLVM_READONLY { return KeywordLoc; }
  SourceLocation getEndLoc() const LLVM_READONLY {
    return SubStmt->getEndLoc();
  }

  child_range children() {
    return child_range(&SubStmt, &SubStmt+1);
  }

  const_child_range children() const {
    return const_child_range(&SubStmt, &SubStmt + 1);
  }

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == MSDependentExistsStmtClass;
  }
};

/// Represents the body of a coroutine. This wraps the normal function
/// body and holds the additional semantic context required to set up and tear
/// down the coroutine frame.
class CoroutineBodyStmt final
    : public Stmt,
      private llvm::TrailingObjects<CoroutineBodyStmt, Stmt *> {
  enum SubStmt {
    Body,          ///< The body of the coroutine.
    Promise,       ///< The promise statement.
    InitSuspend,   ///< The initial suspend statement, run before the body.
    FinalSuspend,  ///< The final suspend statement, run after the body.
    OnException,   ///< Handler for exceptions thrown in the body.
    OnFallthrough, ///< Handler for control flow falling off the body.
    Allocate,      ///< Coroutine frame memory allocation.
    Deallocate,    ///< Coroutine frame memory deallocation.
    ReturnValue,   ///< Return value for thunk function: p.get_return_object().
    ResultDecl,    ///< Declaration holding the result of get_return_object.
    ReturnStmt,    ///< Return statement for the thunk function.
    ReturnStmtOnAllocFailure, ///< Return statement if allocation failed.
    FirstParamMove ///< First offset for move construction of parameter copies.
  };
  unsigned NumParams;

  friend class ASTStmtReader;
  friend class ASTReader;
  friend TrailingObjects;

  Stmt **getStoredStmts() { return getTrailingObjects<Stmt *>(); }

  Stmt *const *getStoredStmts() const { return getTrailingObjects<Stmt *>(); }

public:

  struct CtorArgs {
    Stmt *Body = nullptr;
    Stmt *Promise = nullptr;
    Expr *InitialSuspend = nullptr;
    Expr *FinalSuspend = nullptr;
    Stmt *OnException = nullptr;
    Stmt *OnFallthrough = nullptr;
    Expr *Allocate = nullptr;
    Expr *Deallocate = nullptr;
    Expr *ReturnValue = nullptr;
    Stmt *ResultDecl = nullptr;
    Stmt *ReturnStmt = nullptr;
    Stmt *ReturnStmtOnAllocFailure = nullptr;
    ArrayRef<Stmt *> ParamMoves;
  };

private:

  CoroutineBodyStmt(CtorArgs const& Args);

public:
  static CoroutineBodyStmt *Create(const ASTContext &C, CtorArgs const &Args);
  static CoroutineBodyStmt *Create(const ASTContext &C, EmptyShell,
                                   unsigned NumParams);

  bool hasDependentPromiseType() const {
    return getPromiseDecl()->getType()->isDependentType();
  }

  /// Retrieve the body of the coroutine as written. This will be either
  /// a CompoundStmt or a TryStmt.
  Stmt *getBody() const {
    return getStoredStmts()[SubStmt::Body];
  }

  Stmt *getPromiseDeclStmt() const {
    return getStoredStmts()[SubStmt::Promise];
  }
  VarDecl *getPromiseDecl() const {
    return cast<VarDecl>(cast<DeclStmt>(getPromiseDeclStmt())->getSingleDecl());
  }

  Stmt *getInitSuspendStmt() const {
    return getStoredStmts()[SubStmt::InitSuspend];
  }
  Stmt *getFinalSuspendStmt() const {
    return getStoredStmts()[SubStmt::FinalSuspend];
  }

  Stmt *getExceptionHandler() const {
    return getStoredStmts()[SubStmt::OnException];
  }
  Stmt *getFallthroughHandler() const {
    return getStoredStmts()[SubStmt::OnFallthrough];
  }

  Expr *getAllocate() const {
    return cast_or_null<Expr>(getStoredStmts()[SubStmt::Allocate]);
  }
  Expr *getDeallocate() const {
    return cast_or_null<Expr>(getStoredStmts()[SubStmt::Deallocate]);
  }
  Expr *getReturnValueInit() const {
    return cast<Expr>(getStoredStmts()[SubStmt::ReturnValue]);
  }
  Stmt *getResultDecl() const { return getStoredStmts()[SubStmt::ResultDecl]; }
  Stmt *getReturnStmt() const { return getStoredStmts()[SubStmt::ReturnStmt]; }
  Stmt *getReturnStmtOnAllocFailure() const {
    return getStoredStmts()[SubStmt::ReturnStmtOnAllocFailure];
  }
  ArrayRef<Stmt const *> getParamMoves() const {
    return {getStoredStmts() + SubStmt::FirstParamMove, NumParams};
  }

  SourceLocation getBeginLoc() const LLVM_READONLY {
    return getBody() ? getBody()->getBeginLoc()
                     : getPromiseDecl()->getBeginLoc();
  }
  SourceLocation getEndLoc() const LLVM_READONLY {
    return getBody() ? getBody()->getEndLoc() : getPromiseDecl()->getEndLoc();
  }

  child_range children() {
    return child_range(getStoredStmts(),
                       getStoredStmts() + SubStmt::FirstParamMove + NumParams);
  }

  const_child_range children() const {
    return const_child_range(getStoredStmts(), getStoredStmts() +
                                                   SubStmt::FirstParamMove +
                                                   NumParams);
  }

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == CoroutineBodyStmtClass;
  }
};

/// Represents a 'co_return' statement in the C++ Coroutines TS.
///
/// This statament models the initialization of the coroutine promise
/// (encapsulating the eventual notional return value) from an expression
/// (or braced-init-list), followed by termination of the coroutine.
///
/// This initialization is modeled by the evaluation of the operand
/// followed by a call to one of:
///   <promise>.return_value(<operand>)
///   <promise>.return_void()
/// which we name the "promise call".
class CoreturnStmt : public Stmt {
  SourceLocation CoreturnLoc;

  enum SubStmt { Operand, PromiseCall, Count };
  Stmt *SubStmts[SubStmt::Count];

  bool IsImplicit : 1;

  friend class ASTStmtReader;
public:
  CoreturnStmt(SourceLocation CoreturnLoc, Stmt *Operand, Stmt *PromiseCall,
               bool IsImplicit = false)
      : Stmt(CoreturnStmtClass), CoreturnLoc(CoreturnLoc),
        IsImplicit(IsImplicit) {
    SubStmts[SubStmt::Operand] = Operand;
    SubStmts[SubStmt::PromiseCall] = PromiseCall;
  }

  CoreturnStmt(EmptyShell) : CoreturnStmt({}, {}, {}) {}

  SourceLocation getKeywordLoc() const { return CoreturnLoc; }

  /// Retrieve the operand of the 'co_return' statement. Will be nullptr
  /// if none was specified.
  Expr *getOperand() const { return static_cast<Expr*>(SubStmts[Operand]); }

  /// Retrieve the promise call that results from this 'co_return'
  /// statement. Will be nullptr if either the coroutine has not yet been
  /// finalized or the coroutine has no eventual return type.
  Expr *getPromiseCall() const {
    return static_cast<Expr*>(SubStmts[PromiseCall]);
  }

  bool isImplicit() const { return IsImplicit; }
  void setIsImplicit(bool value = true) { IsImplicit = value; }

  SourceLocation getBeginLoc() const LLVM_READONLY { return CoreturnLoc; }
  SourceLocation getEndLoc() const LLVM_READONLY {
    return getOperand() ? getOperand()->getEndLoc() : getBeginLoc();
  }

  child_range children() {
    if (!getOperand())
      return child_range(SubStmts + SubStmt::PromiseCall,
                         SubStmts + SubStmt::Count);
    return child_range(SubStmts, SubStmts + SubStmt::Count);
  }

  const_child_range children() const {
    if (!getOperand())
      return const_child_range(SubStmts + SubStmt::PromiseCall,
                               SubStmts + SubStmt::Count);
    return const_child_range(SubStmts, SubStmts + SubStmt::Count);
  }

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == CoreturnStmtClass;
  }
};

/// Pattern Matching: PatternStmt is the base class for WildcardPatternStmt, 
/// IdentifierPatternStmt and ExpressionPatternStmt
class PatternStmt : public Stmt {
protected:
  /// The location of the ":".
  SourceLocation ColonLoc;

  /// A pointer to the following PatternStmt class in the same
  /// inspect statement.
  PatternStmt *NextPattern = nullptr;

  PatternStmt(StmtClass SC, SourceLocation KWLoc, SourceLocation ColonLoc)
    : Stmt(SC), ColonLoc(ColonLoc) {
    setPatternLoc(KWLoc);
  }

  PatternStmt(StmtClass SC, EmptyShell) : Stmt(SC) {}

public:
  const PatternStmt *getNextPattern() const { return NextPattern; }
  PatternStmt *getNextPattern() { return NextPattern; }
  void setNextPattern(PatternStmt *SC) { NextPattern = SC; }

  SourceLocation getPatternLoc() const { return InspectPatternBits.PatternLoc; }
  void setPatternLoc(SourceLocation L) { InspectPatternBits.PatternLoc = L; }
  SourceLocation getColonLoc() const { return ColonLoc; }
  void setColonLoc(SourceLocation L) { ColonLoc = L; }

  inline Stmt *getSubStmt();
  const Stmt *getSubStmt() const {
    return const_cast<PatternStmt *>(this)->getSubStmt();
  }

  SourceLocation getBeginLoc() const { return getPatternLoc(); }
  inline SourceLocation getEndLoc() const LLVM_READONLY;

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == WildcardPatternStmtClass ||
      T->getStmtClass() == IdentifierPatternStmtClass ||
      T->getStmtClass() == ExpressionPatternStmtClass;
  }
};

class WildcardPatternStmt final
  : public PatternStmt,
  private llvm::TrailingObjects<WildcardPatternStmt, Stmt *> {
  friend TrailingObjects;

  // WildcardPatternStmt is followed by several trailing objects.
  //
  // * A "Stmt *" for the LHS of the pattern statement. Always present.
  //
  // * A "Stmt *" for the substatement of the pattern statement. Always present.
  enum { LhsOffset = 0, SubStmtOffsetFromLhs = 1 };
  enum { NumMandatoryStmtPtr = 2 };

  unsigned numTrailingObjects(OverloadToken<Stmt *>) const {
    return NumMandatoryStmtPtr;
  }

  unsigned lhsOffset() const { return LhsOffset; }
  unsigned subStmtOffset() const { return lhsOffset() + SubStmtOffsetFromLhs; }

  WildcardPatternStmt(Expr *lhs, SourceLocation patternLoc, SourceLocation colonLoc)
    : PatternStmt(WildcardPatternStmtClass, patternLoc, colonLoc) {
    setLHS(lhs);
    setSubStmt(nullptr);
  }

  /// Build an empty wildcard pattern statement.
  explicit WildcardPatternStmt(EmptyShell Empty)
    : PatternStmt(WildcardPatternStmtClass, Empty) {
  }

public:
  /// Build a wildcard pattern statement.
  static WildcardPatternStmt *Create(const ASTContext &Ctx, Expr *lhs, 
                                     SourceLocation patternLoc, SourceLocation colonLoc);

  /// Build an empty wildcard pattern statement.
  static WildcardPatternStmt *CreateEmpty(const ASTContext &Ctx);

  SourceLocation getIdentifierLoc() const { return getPatternLoc(); }
  void setIdentifierLoc(SourceLocation L) { setPatternLoc(L); }

  Expr *getLHS() {
    return reinterpret_cast<Expr *>(getTrailingObjects<Stmt *>()[lhsOffset()]);
  }

  const Expr *getLHS() const {
    return reinterpret_cast<Expr *>(getTrailingObjects<Stmt *>()[lhsOffset()]);
  }

  void setLHS(Expr *Val) {
    getTrailingObjects<Stmt *>()[lhsOffset()] = reinterpret_cast<Stmt *>(Val);
  }

  Stmt *getSubStmt() { return getTrailingObjects<Stmt *>()[subStmtOffset()]; }
  const Stmt *getSubStmt() const {
    return getTrailingObjects<Stmt *>()[subStmtOffset()];
  }

  void setSubStmt(Stmt *S) {
    getTrailingObjects<Stmt *>()[subStmtOffset()] = S;
  }

  SourceLocation getBeginLoc() const { return getIdentifierLoc(); }
  SourceLocation getEndLoc() const LLVM_READONLY {
    // Handle deeply nested wildcard pattern statements with iteration instead of recursion.
    const PatternStmt *CS = this;
    while (const auto *CS2 = dyn_cast<PatternStmt>(CS->getSubStmt()))
      CS = CS2;

    return CS->getSubStmt()->getEndLoc();
  }

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == WildcardPatternStmtClass;
  }

  // Iterators
  child_range children() {
    return child_range(getTrailingObjects<Stmt *>(),
      getTrailingObjects<Stmt *>() +
      numTrailingObjects(OverloadToken<Stmt *>()));
  }

  const_child_range children() const {
    return const_child_range(getTrailingObjects<Stmt *>(),
      getTrailingObjects<Stmt *>() +
      numTrailingObjects(OverloadToken<Stmt *>()));
  }
};

class IdentifierPatternStmt final
  : public PatternStmt, 
  private llvm::TrailingObjects<IdentifierPatternStmt, Stmt *> {
  friend TrailingObjects;

  // IdentifierPatternStmt is followed by several trailing objects.
  //
  // * A "Stmt *" for the LHS of the pattern statement. Always present.
  //
  // * A "Stmt *" for the substatement of the pattern statement. Always present.
  enum { LhsOffset = 0, SubStmtOffsetFromLhs = 1 };
  enum { NumMandatoryStmtPtr = 2 };

  unsigned numTrailingObjects(OverloadToken<Stmt *>) const {
    return NumMandatoryStmtPtr;
  }

  unsigned lhsOffset() const { return LhsOffset; }
  unsigned subStmtOffset() const { return lhsOffset() + SubStmtOffsetFromLhs; }

  IdentifierPatternStmt(Expr *lhs, SourceLocation patternLoc, SourceLocation colonLoc)
    : PatternStmt(IdentifierPatternStmtClass, patternLoc, colonLoc) {
    setLHS(lhs);
    setSubStmt(nullptr);
  }

  /// Build an empty identifier pattern statement.
  explicit IdentifierPatternStmt(EmptyShell Empty)
    : PatternStmt(IdentifierPatternStmtClass, Empty) {
  }

public:
  /// Build a identifier pattern statement.
  static IdentifierPatternStmt *Create(const ASTContext &Ctx, Expr *lhs, 
                                       SourceLocation patternLoc, SourceLocation colonLoc);

  /// Build an empty identifier pattern statement.
  static IdentifierPatternStmt *CreateEmpty(const ASTContext &Ctx);

  SourceLocation getIdentifierLoc() const { return getPatternLoc(); }
  void setIdentifierLoc(SourceLocation L) { setPatternLoc(L); }

  Expr *getLHS() {
    return reinterpret_cast<Expr *>(getTrailingObjects<Stmt *>()[lhsOffset()]);
  }

  const Expr *getLHS() const {
    return reinterpret_cast<Expr *>(getTrailingObjects<Stmt *>()[lhsOffset()]);
  }

  void setLHS(Expr *Val) {
    getTrailingObjects<Stmt *>()[lhsOffset()] = reinterpret_cast<Stmt *>(Val);
  }

  Stmt *getSubStmt() { return getTrailingObjects<Stmt *>()[subStmtOffset()]; }
  const Stmt *getSubStmt() const {
    return getTrailingObjects<Stmt *>()[subStmtOffset()];
  }

  void setSubStmt(Stmt *S) {
    getTrailingObjects<Stmt *>()[subStmtOffset()] = S;
  }

  SourceLocation getBeginLoc() const { return getIdentifierLoc(); }
  SourceLocation getEndLoc() const LLVM_READONLY {
    // Handle deeply nested identifier pattern statements with iteration instead of recursion.
    const PatternStmt *CS = this;
    while (const auto *CS2 = dyn_cast<PatternStmt>(CS->getSubStmt()))
      CS = CS2;

    return CS->getSubStmt()->getEndLoc();
  }

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == IdentifierPatternStmtClass;
  }

  // Iterators
  child_range children() {
    return child_range(getTrailingObjects<Stmt *>(),
      getTrailingObjects<Stmt *>() +
      numTrailingObjects(OverloadToken<Stmt *>()));
  }

  const_child_range children() const {
    return const_child_range(getTrailingObjects<Stmt *>(),
      getTrailingObjects<Stmt *>() +
      numTrailingObjects(OverloadToken<Stmt *>()));
  }
};

class ExpressionPatternStmt final
  : public PatternStmt,
  private llvm::TrailingObjects<ExpressionPatternStmt, Stmt *> {
  friend TrailingObjects;

  // ExpressionPatternStmt is followed by several trailing objects.
  //
  // * A "Stmt *" for the LHS of the pattern statement. Always present.
  //
  // * A "Stmt *" for the substatement of the pattern statement. Always present.
  enum { LhsOffset = 0, SubStmtOffsetFromLhs = 1 };
  enum { NumMandatoryStmtPtr = 2 };

  unsigned numTrailingObjects(OverloadToken<Stmt *>) const {
    return NumMandatoryStmtPtr;
  }

  unsigned lhsOffset() const { return LhsOffset; }
  unsigned subStmtOffset() const { return lhsOffset() + SubStmtOffsetFromLhs; }

  ExpressionPatternStmt(Expr *lhs, SourceLocation patternLoc, SourceLocation colonLoc)
    : PatternStmt(ExpressionPatternStmtClass, patternLoc, colonLoc) {
    setLHS(lhs);
    setSubStmt(nullptr);
  }

  /// Build an empty expression pattern statement.
  explicit ExpressionPatternStmt(EmptyShell Empty)
    : PatternStmt(ExpressionPatternStmtClass, Empty) {
  }

public:
  /// Build a expression pattern statement.
  static ExpressionPatternStmt *Create(const ASTContext &Ctx, Expr *lhs, 
                                       SourceLocation patternLoc, SourceLocation colonLoc);

  /// Build an empty expression pattern statement.
  static ExpressionPatternStmt *CreateEmpty(const ASTContext &Ctx);

  SourceLocation getIdentifierLoc() const { return getPatternLoc(); }
  void setIdentifierLoc(SourceLocation L) { setPatternLoc(L); }

  Expr *getLHS() {
    return reinterpret_cast<Expr *>(getTrailingObjects<Stmt *>()[lhsOffset()]);
  }

  const Expr *getLHS() const {
    return reinterpret_cast<Expr *>(getTrailingObjects<Stmt *>()[lhsOffset()]);
  }

  void setLHS(Expr *Val) {
    getTrailingObjects<Stmt *>()[lhsOffset()] = reinterpret_cast<Stmt *>(Val);
  }

  Stmt *getSubStmt() { return getTrailingObjects<Stmt *>()[subStmtOffset()]; }
  const Stmt *getSubStmt() const {
    return getTrailingObjects<Stmt *>()[subStmtOffset()];
  }

  void setSubStmt(Stmt *S) {
    getTrailingObjects<Stmt *>()[subStmtOffset()] = S;
  }

  SourceLocation getBeginLoc() const { return getIdentifierLoc(); }
  SourceLocation getEndLoc() const LLVM_READONLY {
    // Handle deeply nested expression pattern statements with iteration instead of recursion.
    const PatternStmt *CS = this;
    while (const auto *CS2 = dyn_cast<PatternStmt>(CS->getSubStmt()))
      CS = CS2;

    return CS->getSubStmt()->getEndLoc();
  }

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == ExpressionPatternStmtClass;
  }

  // Iterators
  child_range children() {
    return child_range(getTrailingObjects<Stmt *>(),
      getTrailingObjects<Stmt *>() +
      numTrailingObjects(OverloadToken<Stmt *>()));
  }

  const_child_range children() const {
    return const_child_range(getTrailingObjects<Stmt *>(),
      getTrailingObjects<Stmt *>() +
      numTrailingObjects(OverloadToken<Stmt *>()));
  }
};

SourceLocation PatternStmt::getEndLoc() const {
  if (const auto *WP = dyn_cast<WildcardPatternStmt>(this))
    return WP->getEndLoc();
  else if (const auto *IP = dyn_cast<IdentifierPatternStmt>(this))
    return IP->getEndLoc();
  else if (const auto *EP = dyn_cast<ExpressionPatternStmt>(this))
    return EP->getEndLoc();

  llvm_unreachable("PatternStmt is neither a WildcardPatternStmt nor"
                   "a IdentifierPatternStmt, nor a ExpressionPatternStmt!");
}

Stmt *PatternStmt::getSubStmt() {
  if (auto *WP = dyn_cast<WildcardPatternStmt>(this))
    return WP->getSubStmt();
  else if (auto *IP = dyn_cast<IdentifierPatternStmt>(this))
    return IP->getSubStmt();
  else if (auto *EP = dyn_cast<ExpressionPatternStmt>(this))
    return EP->getSubStmt();

  llvm_unreachable("PatternStmt is neither a WildcardPatternStmt nor"
    "a IdentifierPatternStmt, nor a ExpressionPatternStmt!");
}

/// InspectStmt - This represents an 'inspect' stmt.
class InspectStmt final : public Stmt,
                          private llvm::TrailingObjects<InspectStmt, Stmt *> {
  friend TrailingObjects;

  /// Points to a linked list of patterns
  PatternStmt *FirstPattern;

  // InspectStmt is followed by several trailing objects,
  // some of which optional. Note that it would be more convenient to
  // put the optional trailing objects at the end but this would change
  // the order in children().
  // The trailing objects are in order:
  //
  // * A "Stmt *" for the init statement.
  //    Present if and only if hasInitStorage().
  //
  // * A "Stmt *" for the condition variable.
  //    Present if and only if hasVarStorage(). This is in fact a "DeclStmt *".
  //
  // * A "Stmt *" for the condition.
  //    Always present. This is in fact an "Expr *".
  //
  // * A "Stmt *" for the body.
  //    Always present.
  enum { InitOffset = 0, BodyOffsetFromCond = 1 };
  enum { NumMandatoryStmtPtr = 2 };

  unsigned numTrailingObjects(OverloadToken<Stmt *>) const {
    return NumMandatoryStmtPtr + hasInitStorage() + hasVarStorage();
  }

  unsigned initOffset() const { return InitOffset; }
  unsigned varOffset() const { return InitOffset + hasInitStorage(); }
  unsigned condOffset() const {
    return InitOffset + hasInitStorage() + hasVarStorage();
  }
  unsigned bodyOffset() const { return condOffset() + BodyOffsetFromCond; }

  /// Build an inspect statement.
  InspectStmt(const ASTContext &Ctx, Stmt *Init, VarDecl *Var,
              Expr *Cond);

  /// Build an empty inspect statement.
  explicit InspectStmt(EmptyShell Empty, bool HasInit,
                                  bool HasVar);

public:
  /// Create an inspect statement.
  static InspectStmt *Create(const ASTContext &Ctx, Stmt *Init, VarDecl *Var,
                             Expr *Cond);

  /// Create an empty inspect statement optionally with storage for
  /// an init expression and a condition variable.
  static InspectStmt *CreateEmpty(const ASTContext &Ctx, bool HasInit,
                                  bool HasVar);

  /// True if this SwitchStmt has storage for an init statement.
  bool hasInitStorage() const { return InspectStmtBits.HasInit; }

  /// True if this SwitchStmt has storage for a condition variable.
  bool hasVarStorage() const { return InspectStmtBits.HasVar; }

  Expr *getCond() {
    return reinterpret_cast<Expr *>(getTrailingObjects<Stmt *>()[condOffset()]);
  }

  const Expr *getCond() const {
    return reinterpret_cast<Expr *>(getTrailingObjects<Stmt *>()[condOffset()]);
  }

  void setCond(Expr *Cond) {
    getTrailingObjects<Stmt *>()[condOffset()] = reinterpret_cast<Stmt *>(Cond);
  }

  Stmt *getBody() { return getTrailingObjects<Stmt *>()[bodyOffset()]; }
  const Stmt *getBody() const {
    return getTrailingObjects<Stmt *>()[bodyOffset()];
  }

  void setBody(Stmt *Body) {
    getTrailingObjects<Stmt *>()[bodyOffset()] = Body;
  }

  Stmt *getInit() {
    return hasInitStorage() ? getTrailingObjects<Stmt *>()[initOffset()]
      : nullptr;
  }

  const Stmt *getInit() const {
    return hasInitStorage() ? getTrailingObjects<Stmt *>()[initOffset()]
      : nullptr;
  }

  void setInit(Stmt *Init) {
    assert(hasInitStorage() &&
      "This inspect statement has no storage for an init statement!");
    getTrailingObjects<Stmt *>()[initOffset()] = Init;
  }

  /// Retrieve the variable declared in this "inspect" statement, if any.
  ///
  /// In the following example, "x" is the condition variable.
  /// \code
  /// switch (int x = foo()) {
  ///   case 0: break;
  ///   // ...
  /// }
/// \endcode
  VarDecl *getConditionVariable();
  const VarDecl *getConditionVariable() const {
    return const_cast<InspectStmt *>(this)->getConditionVariable();
  }

  /// Set the condition variable in this switch statement.
  /// The switch statement must have storage for it.
  void setConditionVariable(const ASTContext &Ctx, VarDecl *VD);

  /// If this SwitchStmt has a condition variable, return the faux DeclStmt
  /// associated with the creation of that condition variable.
  DeclStmt *getConditionVariableDeclStmt() {
    return hasVarStorage() ? static_cast<DeclStmt *>(
      getTrailingObjects<Stmt *>()[varOffset()])
      : nullptr;
  }

  const DeclStmt *getConditionVariableDeclStmt() const {
    return hasVarStorage() ? static_cast<DeclStmt *>(
      getTrailingObjects<Stmt *>()[varOffset()])
      : nullptr;
  }

  PatternStmt *getPatternList() { return FirstPattern; }
  const PatternStmt *getPatternList() const { return FirstPattern; }
  void setPatternList(PatternStmt *SC) { FirstPattern = SC; }

  SourceLocation getInspectLoc() const { return InspectStmtBits.InspectLoc; }
  void setInspectLoc(SourceLocation L) { InspectStmtBits.InspectLoc = L; }

  void setBody(Stmt *S, SourceLocation SL) {
    setBody(S);
    setInspectLoc(SL);
  }

  void addPattern(PatternStmt *SC) {
     assert(!SC->getNextPattern() &&
       "pattern already added to an inspect");

     // note this logic was inherited from switch
     // where the order of traversal of case/default
     // statements doesn't matter. May have to revisit
     // this to maintain the linked list in insertion
     // order
     SC->setNextPattern(FirstPattern);
     FirstPattern = SC;
  }

  SourceLocation getBeginLoc() const { return getInspectLoc(); }
  SourceLocation getEndLoc() const LLVM_READONLY {
    return getBody() ? getBody()->getEndLoc()
      : reinterpret_cast<const Stmt *>(getCond())->getEndLoc();
  }

  // Iterators
  child_range children() {
    return child_range(getTrailingObjects<Stmt *>(),
      getTrailingObjects<Stmt *>() +
      numTrailingObjects(OverloadToken<Stmt *>()));
  }

  const_child_range children() const {
    return const_child_range(getTrailingObjects<Stmt *>(),
      getTrailingObjects<Stmt *>() +
      numTrailingObjects(OverloadToken<Stmt *>()));
  }

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == InspectStmtClass;
  }
};
}  // end namespace clang

#endif
