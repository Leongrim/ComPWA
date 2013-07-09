//! FunctionTree for the actual Optimization
/*! \class FunctionTree
 * @file FunctionTree.hpp
 * This class can be used to store a function in a tree-like structure. This
 * reduces the amount of recalculation needed when performing an optimization,
 * as most often only parts need to be recalculated in one iteration.
*/

#ifndef _FUNCTIONTREE_HPP_
#define _FUNCTIONTREE_HPP_

#include <vector>
#include <memory>
#include <string>

#include "Core/Functions.hpp"
#include "Core/TreeNode.hpp"
#include "Core/AbsParameter.hpp"
#include "Core/Parameter.hpp"

class FunctionTree
{
public:
  //! Standard constructor
   /*!
    * Standard constructor for empty tree
   */
  FunctionTree(){

  }

  //! Standard constructor
   /*!
    * Standard constructor with the top node provided
    * /param head first node to be used as head
   */
  FunctionTree(std::shared_ptr<TreeNode> head):head_(head){
    nodes_.insert(std::pair<std::string, std::shared_ptr<TreeNode> >(head->getName(),head));
  }

  //! Destructor
  virtual ~FunctionTree(){
    //;
  }

  //! Add node to FcnTree
   /*!
    * Add a node to the function tree
    * Adds Top-Down-Linking to the node
    * \param newNode Node to be added
   */
  virtual void addNode(std::shared_ptr<TreeNode> newNode){
    //TODO: check existence, throw exception
    nodes_.insert(std::pair<std::string, std::shared_ptr<TreeNode> >(newNode->getName(),newNode));
    newNode->linkParents();
  }

  //! Create head node of FcnTree
   /*!
    * Add top node to the function tree
    * \param name identifier of node
    * \param strat Strategy with which the node calculates its value
    * \sa addNode(), createNode(), createLeaf()
   */
  virtual void createHead(const std::string& name, std::shared_ptr<Strategy> strat){
    //TODO: (type of) parameter from strategy!
    //std::shared_ptr<AbsParameter> inter = strat->GetResultContainer();

    std::shared_ptr<DoubleParameter> inter(new DoubleParameter("par"+name,0.));
    std::shared_ptr<TreeNode> newNode(new TreeNode(name, inter, strat, NULL));
    head_ = newNode;
    nodes_.insert(std::pair<std::string, std::shared_ptr<TreeNode> >(name,newNode));
  }

  //! Create a node for the FcnTree
   /*!
    * Create and add a node to the function tree
    * Adds Top-Down-Linking to the node
    * \param name identifier of node
    * \param strat Strategy with which the node calculates its value
    * \param parent the parent of this node (for linking)
    * \sa addNode(), createHead(), createLeaf()
   */
  virtual void createNode(const std::string& name, std::shared_ptr<Strategy> strat, std::string parent){
    //TODO: (type of) parameter from strategy!
    //std::shared_ptr<AbsParameter> inter = strat->GetResultContainer();

    std::shared_ptr<DoubleParameter> inter(new DoubleParameter("par"+name,0.));
    std::shared_ptr<TreeNode> parentNode = nodes_.at(parent);
    std::shared_ptr<TreeNode> newNode(new TreeNode(name, inter, strat, parentNode));
    nodes_.insert(std::pair<std::string, std::shared_ptr<TreeNode> >(name,newNode));
    newNode->linkParents();
  }

  //! Create a leaf for the FcnTree
   /*!
    * Create and add a node to the function tree
    * Adds Top-Down-Linking to the node
    * Attaches the Node as Observer to the external parameter
    * \param name identifier of node
    * \param extPar the parameter this node represents
    * \param parent the parent of this node (for linking)
    * \sa addNode(), createHead(), createNode()
   */
  virtual void createLeaf(const std::string& name, std::shared_ptr<AbsParameter> extPar, std::string parent){
    std::shared_ptr<TreeNode> parentNode = nodes_.at(parent);
    std::shared_ptr<TreeNode> newNode(new TreeNode(name, extPar, NULL, parentNode));
    nodes_.insert(std::pair<std::string, std::shared_ptr<TreeNode> >(name,newNode));
    newNode->linkParents();
    extPar->Attach(newNode);
  }

  //! return the head of the tree
   /*!
    * Access to the head element of the tree
    * \return FuntionTreeNode at head of tree
   */
  virtual const std::shared_ptr<TreeNode> head() const {
    //fcnTree_.
    return head_;
    //TODO: return double? ;
  }

  void recalculate(){
    head_->recalculate();
  }

  //! friend function to stream parameter information to output
  /*!
   * Declaring the stream-operator << as friend allows to stream parameter
   * information to the output as easily as a generic type.
   * \sa make_str(), to_str()
  */
  friend std::ostream& operator<<( std::ostream& out, const FunctionTree& b ){
    return out << b.head();
  }

  //! friend function to stream parameter information to output
  /*!
   * Declaring the stream-operator << as friend allows to stream parameter
   * information to the output as easily as a generic type.
   * \param out the ostream the object is piped to
   * \param b the piped object
   * \sa make_str(), to_str()
  */
  friend std::ostream& operator<<( std::ostream& out, std::shared_ptr<FunctionTree> b ){
    return out << b->head();
  }


protected:
  std::shared_ptr<TreeNode> head_; /*!< the head node storing the absolute result */
  std::map<std::string, std::shared_ptr<TreeNode> > nodes_; /*!< map to store the nodes */

};

#endif /* _FUNCTIONTREE_HPP_ */