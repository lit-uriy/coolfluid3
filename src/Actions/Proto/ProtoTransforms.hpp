// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Actions_ProtoTransforms_hpp
#define CF_Actions_ProtoTransforms_hpp

#include <boost/fusion/container/list/cons.hpp>
#include <boost/fusion/include/cons.hpp>

#include <boost/mpl/max.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/mpl/void.hpp>

#include "Actions/Proto/ProtoVariables.hpp"

#include "Math/MatrixTypes.hpp"
#include "Mesh/CTable.hpp"

#include <boost/variant/variant.hpp>

namespace boost {
template<class T >
struct remove_reference;
}

namespace CF {
namespace Actions {
namespace Proto {

template<typename T>
struct VarArity
{
  typedef typename T::index_type type;
};

/// Gets the arity (max index) for the numbered variables
struct ExprVarArity
  : boost::proto::or_<
        boost::proto::when< boost::proto::terminal< Var<boost::proto::_, boost::proto::_> >,
          boost::mpl::next< VarArity<boost::proto::_value> >()
        >
      , boost::proto::when< boost::proto::terminal<boost::proto::_>,
          boost::mpl::int_<0>()
        >
      , boost::proto::when<
            boost::proto::nary_expr<boost::proto::_, boost::proto::vararg<boost::proto::_> >
          , boost::proto::fold<boost::proto::_, boost::mpl::int_<0>(), boost::mpl::max<ExprVarArity, boost::proto::_state>()>
        >
    >
{};
  
/// Functor for evaluting an expression using proto::eval
struct ContextEvaluator :
  boost::proto::transform< ContextEvaluator >
{
  template<typename Expr, typename State, typename Data>
  struct impl : boost::proto::transform_impl<Expr, State, Data>
  {
    typedef typename boost::proto::result_of::eval
    <
      typename boost::remove_const
      <
        typename boost::remove_reference<Expr>::type
      >::type,
      typename boost::remove_const
      <
        typename boost::remove_reference<Data>::type
      >::type
    >::type result_type;
  
    result_type operator ()(
                typename impl::expr_param expr
              , typename impl::state_param state
              , typename impl::data_param data
    ) const
    {
      return boost::proto::eval(expr, data);
    }
  };
};

/// Handle assignment to an LSS matrix
template<typename OpT>
struct BlockAccumulator :
  boost::proto::transform< BlockAccumulator<OpT> >
{
  template<typename Expr, typename State, typename ContextT>
  struct impl : boost::proto::transform_impl<Expr, State, ContextT>
  {
    typedef void result_type;
    
    /// Number of the numbered varaible that determines the connectivity. We assume there is only one numbered var here.
    typedef typename boost::mpl::prior<typename boost::result_of<ExprVarArity(typename boost::proto::result_of::right<Expr>::type)>::type>::type I;
    typedef typename boost::proto::result_of::value<typename boost::proto::result_of::left<Expr>::type>::type MatrixHolderT;
    typedef typename boost::proto::result_of::eval
    <
      typename boost::remove_reference<typename boost::proto::result_of::right<Expr>::type>::type,
      typename boost::remove_reference<ContextT>::type
    >::type RhsT;
  
    result_type operator ()(
                typename impl::expr_param expr
              , typename impl::state_param state
              , typename impl::data_param context // We assume data is a context
    ) const
    {
      const Mesh::CTable::ConstRow conn_row = (*boost::fusion::at<I>(context.contexts).connectivity)[context.element_idx];
      MatrixHolderT m = boost::proto::value(boost::proto::left(expr));
      BlockAssignmentOp<OpT>::assign(m.matrix(), boost::proto::eval(boost::proto::right(expr), context), conn_row);
    }
  };
};

struct DirichletBCSetter :
  boost::proto::transform< DirichletBCSetter >
{
  template<typename Expr, typename State, typename ContextT>
  struct impl : boost::proto::transform_impl<Expr, State, ContextT>
  {
    typedef void result_type;
    
    typedef typename boost::proto::result_of::value<typename boost::proto::result_of::left<Expr>::type>::type DirichletBCT;
    typedef typename boost::proto::result_of::eval
    <
      typename boost::remove_reference<typename boost::proto::result_of::right<Expr>::type>::type,
      typename boost::remove_reference<ContextT>::type
    >::type RhsT;
  
    result_type operator ()(
                typename impl::expr_param expr
              , typename impl::state_param state
              , typename impl::data_param context // We assume data is a context
    ) const
    {
      DirichletBCT bc = boost::proto::value(boost::proto::left(expr));
      assign_dirichlet(bc.matrix(), bc.rhs(), boost::proto::eval(boost::proto::right(expr), context), context.node_idx);
    }
  };
};

/// Returns the value of a variable
struct VarValue :
  boost::proto::transform< VarValue >
{
  template<typename VarT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<VarT, StateT, DataT>
  { 
    typedef typename boost::remove_reference<VarT>::type::index_type I;
    typedef typename boost::remove_reference<DataT>::type::template DataType<I>::type VarDataT;
    typedef typename VarDataT::value_type result_type;
  
    result_type operator ()(
                typename impl::expr_param var
              , typename impl::state_param state
              , typename impl::data_param data
    ) const
    {
      return data.template var_data<I>().value();
    }
  };
};

} // Proto
} // Actions
} // CF

namespace boost {
namespace proto {
  /// Make our custom primitive transforms callable
  template<>
  struct is_callable<CF::Actions::Proto::ContextEvaluator>
    : mpl::true_
  {};
  
  template<typename OpT>
  struct is_callable< CF::Actions::Proto::BlockAccumulator<OpT> >
    : mpl::true_
  {};
  
  template<>
  struct is_callable< CF::Actions::Proto::VarValue >
    : mpl::true_
  {};
}
}

namespace CF {
namespace Actions {
namespace Proto {

/*
template<typename LeftT, typename RightT>
struct MultExpr
{
  typedef typename Eigen::ProductReturnType< Eigen::NestByValue<LeftT>, Eigen::NestByValue<RightT> >::Type type;
};

template<typename LeftDerivedT, typename RightDerivedT>
struct MultExpr< Eigen::MatrixBase<LeftDerivedT>, Eigen::MatrixBase<RightDerivedT> >
{
  typedef typename Eigen::ProductReturnType< LeftDerivedT, RightDerivedT >::Type type;
};

struct EvaluateExpr :
  boost::proto::or_
  <
    boost::proto::when
    <
      boost::proto::multiplies<boost::proto::_, boost::proto::_>,
      MultExpr
      <
        EvaluateExpr(boost::proto::_left), 
        EvaluateExpr(boost::proto::_right)
      >(EvaluateExpr(boost::proto::_left), EvaluateExpr(boost::proto::_right))
    >,
    boost::proto::when
    <
      boost::proto::_,
      ContextEvaluator
    >
  >
{};
*/

/// Allowed block assignment operations
struct MatrixAssignOpsCases
{
  template<typename TagT> struct case_  : boost::proto::not_<boost::proto::_> {};
};

template<>
struct MatrixAssignOpsCases::case_<boost::proto::tag::assign> :
boost::proto::when
<
  boost::proto::assign<boost::proto::terminal< Var< boost::proto::_, EigenDenseMatrix<boost::proto::_> > >, boost::proto::_>,
  BlockAccumulator<boost::proto::tag::assign>
> 
{};
template<>
struct MatrixAssignOpsCases::case_<boost::proto::tag::plus_assign> :
boost::proto::when
<
  boost::proto::plus_assign<boost::proto::terminal< Var< boost::proto::_, EigenDenseMatrix<boost::proto::_> > >, boost::proto::_>,
  BlockAccumulator<boost::proto::tag::plus_assign>
> 
{};
template<>
struct MatrixAssignOpsCases::case_<boost::proto::tag::minus_assign> :
boost::proto::when
<
  boost::proto::minus_assign<boost::proto::terminal< Var< boost::proto::_, EigenDenseMatrix<boost::proto::_> > >, boost::proto::_>,
  BlockAccumulator<boost::proto::tag::minus_assign>
> 
{};
template<>
struct MatrixAssignOpsCases::case_<boost::proto::tag::multiplies_assign> :
boost::proto::when
<
  boost::proto::multiplies_assign<boost::proto::terminal< Var< boost::proto::_, EigenDenseMatrix<boost::proto::_> > >, boost::proto::_>,
  BlockAccumulator<boost::proto::tag::multiplies_assign>
> 
{};
template<>
struct MatrixAssignOpsCases::case_<boost::proto::tag::divides_assign> :
boost::proto::when
<
  boost::proto::divides_assign<boost::proto::terminal< Var< boost::proto::_, EigenDenseMatrix<boost::proto::_> > >, boost::proto::_>,
  BlockAccumulator<boost::proto::tag::divides_assign>
> 
{};

/// Executes expressions, delegating to specific transforms when needed
struct EvaluateExpr :
  boost::proto::or_
  <
    boost::proto::switch_<MatrixAssignOpsCases>, // Assignment to system matrix
    boost::proto::when // Assignment to Dirichlet BC
    <
      boost::proto::assign<boost::proto::terminal< Var< boost::proto::_, EigenDenseDirichletBC<boost::proto::_, boost::proto::_> > >, boost::proto::_>,
      DirichletBCSetter
    >,
    boost::proto::when
    <
      boost::proto::_,
      ContextEvaluator
    >
  >
{};

/// Transform that extracts the type of variable I
template<Uint I>
struct DefineType
  : boost::proto::or_<
      boost::proto::when< boost::proto::terminal< Var<boost::mpl::int_<I>, boost::proto::_> >,
          boost::proto::_value
      >
    , boost::proto::when< boost::proto::terminal< boost::proto::_ >,
          boost::mpl::void_()
      >
    , boost::proto::when< boost::proto::nary_expr<boost::proto::_, boost::proto::vararg<boost::proto::_> >
            , boost::proto::fold<boost::proto::_, boost::mpl::void_(), boost::mpl::if_<boost::mpl::is_void_<boost::proto::_state>, DefineType<I>, boost::proto::_state>() >
      >
  >
{};

/// Ease application of DefineType as an MPL lambda expression, and strip the Var encapsulation
template<typename I, typename Expr>
struct DefineTypeOp
{
  typedef typename boost::result_of<DefineType<I::value>(Expr)>::type var_type;
  typedef typename boost::mpl::if_<boost::mpl::is_void_<var_type>, boost::mpl::void_, typename var_type::type>::type type;
};

/// Copy the terminal values to a fusion list
template<typename VarsT>
struct CopyNumberedVars
  : boost::proto::callable_context< CopyNumberedVars<VarsT>, boost::proto::null_context >
{
  typedef void result_type;
  
  CopyNumberedVars(VarsT& vars) : m_vars(vars) {}

  template<typename I, typename T>
  void operator()(boost::proto::tag::terminal, Var<I, T> val)
  {
    boost::fusion::at<I>(m_vars) = val;
  }
  
private:
  VarsT& m_vars;  
};

/// Extract the real value type of a given type, which might be an Eigen expression
template<typename T>
struct ValueType
{
  typedef typename Eigen::MatrixBase<T>::PlainObject type;
  
  static void set_zero(type& val)
  {
    val.setZero();
  }
};

/// Specialize for Eigen matrices
template<int I, int J>
struct ValueType< Eigen::Matrix<Real, I, J> >
{
  typedef Eigen::Matrix<Real, I, J> type;
  
  static void set_zero(type& val)
  {
    val.setZero();
  }
};

/// Specialise for reals
template<>
struct ValueType<Real>
{
  typedef Real type;
  
  static void set_zero(type& val)
  {
    val = 0.;
  }
};

/// Internal
template<typename ExprT, typename MatrixT>
struct Transform1x1MatrixToScalarHelper
{
  typedef ExprT type;
};

/// Internal
template<typename ExprT>
struct Transform1x1MatrixToScalarHelper< ExprT, Eigen::Matrix<Real, 1, 1> >
{
  typedef Real type;
};

/// Turn 1x1 matrices to scalar
template<typename T>
struct Transform1x1MatrixToScalar
{
  typedef typename Transform1x1MatrixToScalarHelper< T, typename Eigen::MatrixBase<T>::PlainObject >::type type;
};

//////////////////////// Testing ///////////////////

struct TestGrammar :
  boost::proto::or_
  <
    boost::proto::when
    <
      boost::proto::terminal< Var< boost::proto::_, boost::proto::_ > >,
      VarValue(boost::proto::_value)
    >,
    boost::proto::when
    <
      boost::proto::_,
      boost::proto::_default<TestGrammar>
    >
  >
{
};

} // Proto
} // Actions
} // CF

#endif // CF_Actions_ProtoTransforms_hpp
