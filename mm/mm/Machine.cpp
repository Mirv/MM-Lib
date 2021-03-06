/******************************************************************************
 * Copyright (c) 2013-2014, Amsterdam University of Applied Sciences (HvA) and
 *                          Centrum Wiskunde & Informatica (CWI)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Contributors:
 *   * Riemer van Rozen - rozen@cwi.nl - HvA / CWI
 ******************************************************************************/
//
//  Machine.cpp
//  mm
//
//  Created by Riemer van Rozen on 7/16/13.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "YYLTYPE.h"
#include "Types.h"
#include "Recyclable.h"
#include "Vector.h"
#include "Map.h"
#include "Recycler.h"
#include "Observer.h"
#include "Observable.h"
#include "Location.h"
#include "String.h"
#include "Name.h"
#include "Element.h"
#include "Transformation.h"
#include "Program.h"
#include "Modification.h"
#include "Transition.h"
#include "Event.h"
#include "FlowEvent.h"
#include "TriggerEvent.h"
#include "Failure.h"
#include "Enablement.h"
#include "Disablement.h"
#include "Activation.h"
#include "Violation.h"
#include "Prevention.h"
#include "Operator.h"
#include "Exp.h"
#include "VarExp.h"
#include "Assertion.h"
#include "Deletion.h"
#include "Edge.h"
#include "StateEdge.h"
#include "FlowEdge.h"
#include "NodeWorkItem.h"
#include "NodeBehavior.h"
#include "Node.h"
#include "PoolNodeBehavior.h"
#include "SourceNodeBehavior.h"
#include "DrainNodeBehavior.h"
#include "GateNodeBehavior.h"
#include "RefNodeBehavior.h"
#include "ConverterNodeBehavior.h"
#include "Declaration.h"
#include "InterfaceNode.h"
#include "Definition.h"
#include "Instance.h"
#include "PoolNodeInstance.h"
#include "Operator.h"
#include "ValExp.h"
#include "UnExp.h"
#include "BinExp.h"
#include "DieExp.h"
#include "RangeValExp.h"
#include "BooleanValExp.h"
#include "NumberValExp.h"
#include "OverrideExp.h"
#include "ActiveExp.h"
#include "AllExp.h"
#include "AliasExp.h"
#include "OneExp.h"
#include "Reflector.h"
#include "Evaluator.h"
#include "Machine.h"

const MM::UINT32 MM::Machine::LOG_SIZE = 1024 * 32 * 8;

MM::Machine::Machine() : MM::Recycler()
{
  reflector = new MM::Reflector(this);
  evaluator = new MM::Evaluator(this);
  log = createString(MM::Machine::LOG_SIZE);
  
  MM::Vector<MM::Element*> * elements = createElementVector();
  type = createDefinition(MM_NULL, elements);
  
  //create the initial state
  inst = createInstance(MM_NULL, type, MM_NULL);

  //let the state observe the global scope type
  type->addObserver(inst);
  
  //initialize the global scope type
  reflector->init(type);
  
  delegates = new MM::Vector<MM::Machine::Delegate *>();
  
  //initialize the start state (set active nodes)
  //evaluator->initStartState(inst);
}

MM::Machine::~Machine()
{
  delete reflector;
  delete delegates;
  evaluator->recycle(this);
  inst->recycle(this);
  log->recycle(this);
  type->recycle(this);
}

MM::TID MM::Machine::getTypeId()
{
  return MM::T_Machine;
}

MM::BOOLEAN MM::Machine::instanceof(MM::TID tid)
{
  if(tid == MM::T_Machine)
  {
    return MM_TRUE;
  }
  else
  {
    return MM::Recycler::instanceof(tid);
  }
}

extern MM::Program * MM_parse(MM::Machine * machine, const MM::CHAR * input);
extern MM::Program * MM_parseFile(MM::Machine * machine, const MM::CHAR * input);

MM::Reflector * MM::Machine::getReflector()
{
  return reflector;
}

MM::Evaluator * MM::Machine::getEvaluator()
{
  return evaluator;
}

MM::Definition * MM::Machine::getDefinition()
{
  return type;
}

MM::Instance * MM::Machine::getInstance()
{
  return inst;
}

MM::UINT32 MM::Machine::getInstance(MM::UINT32 instance, //0 -> global scope
                                    MM::CHAR  * name)
{
  MM::Instance * r = 0;
  
  if(instance == 0)
  {
    r = this->inst;
  }
  else
  {
    MM::Instance * i = (MM::Instance *) instance;
    MM::Definition * def = i->getDefinition();
    
    MM::Name * n = createName(name);
    MM::Element * e = def->getElement(n);
    n->recycle(this);
    
    if(e->instanceof(MM::T_Declaration) == MM_TRUE)
    {
      MM::Declaration * decl = (MM::Declaration *) e;
      r = i->getInstance(decl);
    }
  }
  return (MM::UINT32) r;
}

MM::VOID MM::Machine::getName (MM::UINT32   element,
                               MM::CHAR *   buffer,
                               MM::UINT32   bufferSize)
{
  MM::Element * e = (MM::Element *) element;
  MM::Name * n = e->getName();
  if(n != MM_NULL)
  {
    MM::CHAR * buf = n->getBuffer();
    MM::UINT32 len = n->getLength();
    if(len <= bufferSize)
    {
      snprintf(buffer, bufferSize, "%s", buf);
    }
  }
}

MM::VOID MM::Machine::getInstanceName(MM::UINT32 instance,
                                      MM::CHAR * buffer,
                                      MM::UINT32 bufferSize)
{
  MM::Instance * i = (MM::Instance *) instance;
  MM::String * name = new MM::String(buffer, bufferSize);
  i->nameToString(name);  
  delete name;
}

MM::BOOLEAN MM::Machine::activate(MM::UINT32 node,
                               MM::UINT32 instance)
{
  MM::Node * n = (MM::Node *) node;
  MM::Instance * i = (MM::Instance *) instance;
  MM::BOOLEAN isDisabled = n->isDisabled(i, evaluator, this);

  if(isDisabled == MM_FALSE)
  {
    i->setActive(n);
  }

  return isDisabled;
}

MM::VOID MM::Machine::step()
{
  MM::Transition * tr = createTransition();
  evaluator->step(tr);
  tr->recycle(this);
}

//Query the value of node instances directly
//NOTE: bad practice, this is an event driven system
MM::INT32 MM::Machine::getAmount(MM::UINT32 node, 
                                 MM::UINT32 instance)
{
  MM::Node * n = (MM::Node *) node;
  MM::Instance * i = (MM::Instance *) instance;
  return n->getAmount(i, this);
}

//Set value in nodes directly
//NOTE: node must be extern
MM::VOID MM::Machine::setAmount(MM::UINT32 node, 
                                MM::UINT32 instance,
                                MM::INT32 val)
{
  MM::Node * n = (MM::Node *) node;
  MM::Instance * i = (MM::Instance *) instance;
  n->setAmount(i, this, val);
}

/*
MM::VOID MM::Machine::step (MM::CHAR * buf,
                            MM::UINT32 size)
{
  MM::Transition * tr = createTransition();
  evaluator->step(tr);
  MM::String * str = new MM::String(buf, size);
  tr->toString(str);  
  delete str;
  tr->recycle(this);
}

MM::VOID MM::Machine::step (MM::UINT32 instance,
                            MM::CHAR * buf,
                            MM::UINT32 size)
{
  MM::Instance * i = (MM::Instance *) instance;  
  MM::Transition * tr = createTransition();
  MM::String * str = new MM::String(buf, size);
  evaluator->step(i, tr);
  tr->toString(str);
  delete str;
  tr->recycle(this);
}
*/

MM::UINT32 MM::Machine::addInstanceObserver(MM::UINT32 instance, MM::UINT32 caller, MM::CALLBACK callback)
{
  MM::Instance * i = (MM::Instance *) instance;
  MM::Machine::Delegate * io =
  new MM::Machine::Delegate(i, caller, callback);  
  delegates->add(io);
  return (MM::UINT32) io;
}

MM::UINT32 MM::Machine::addDefinitionObserver(MM::UINT32 definition, MM::UINT32 caller, MM::CALLBACK callback)
{
  MM::Definition * def = (MM::Definition *) definition;
  MM::Machine::Delegate * io =
  new MM::Machine::Delegate(def, caller, callback);  
  delegates->add(io);
  return (MM::UINT32) io;
}

MM::VOID MM::Machine::removeObserver (MM::UINT32 observer)
{
  MM::Machine::Delegate * io =
    (MM::Machine::Delegate *) observer;
  delegates->remove(io);
  delete io;
}


MM::String * MM::Machine::getLog()
{
  return log;
}

MM::VOID MM::Machine::eval (const MM::CHAR * input)
{
  MM::Program * program = MM_parse(this, input);
  if(program != MM_NULL)
  {
    reflect(program);
    program->recycle(this);
  }
}

MM::VOID MM::Machine::evalFile (const MM::CHAR * file)
{
  MM::Program * program = MM_parseFile(this, file);
  if(program != MM_NULL)
  {
    reflect(program);
    program->recycle(this);
  }
}

MM::VOID MM::Machine::reflect(MM::Program * program)
{
  if(program != MM_NULL)
  {
    MM::String * buf = createString(1024 * 100 * 32);
    program->toString(log); //store history
  
    MM::Vector<Transformation *> * ts = program->getTransformations();
    MM::Vector<Transformation *>::Iterator i = ts->getIterator();
    while(i.hasNext() == MM_TRUE)
    {
      Transformation * t = i.getNext();
      if(t->instanceof(MM::T_Modification) == MM_TRUE)
      {
        reflector->merge((MM::Modification *) t);
        MM::Definition * def = reflector->getDefinition();
        buf->clear();
        def->toString(buf);
        buf->print();
      }
    }

    inst->notifyValues(this);

    buf->recycle(this);
  }
}


//MM::VOID MM::Machine::setTree (MM::Definition * def)
//{
  //this->def = def;
  //}

//MM::Element * MM::Machine::getTree()
//{
  //return def;
  //}

//MM::Definition * MM::Machine::getDefinition(MM::Name * name)
//{
//  return types->get(name);
//}

//MM::VOID MM::Machine::putDefinition(MM::Name * name, MM::Definition * def)
//{
  //types->put(name, def);
  //}

MM::Vector<MM::Transformation *> * MM::Machine::createTransformationVector()
{
  MM::Vector<MM::Transformation *> * v = new Vector<MM::Transformation *> ();
  //TODO: process vectors in recycler
  return v;
}

MM::Vector<MM::Element *> * MM::Machine::createElementVector()
{
  MM::Vector<MM::Element *> * v = new MM::Vector<MM::Element*> ();
  //TODO: process vectors in recycler
  return v;
}

MM::Vector<MM::Node *> * MM::Machine::createNodeVector()
{
  MM::Vector<MM::Node *> * v = new MM::Vector<MM::Node *> (); 
  //TODO: process vectors in recycler
  return v;
}

MM::Vector<MM::Edge *> * MM::Machine::createEdgeVector()
{
  MM::Vector<MM::Edge *> * v = new MM::Vector<MM::Edge *> ();
  //TODO: process vectors in recycler
  return v;  
}

MM::Map<MM::Name *, MM::Element *, MM::Name::Compare> * MM::Machine::createName2ElementMap()
{
  MM::Map<MM::Name *, MM::Element *, MM::Name::Compare> * n2e =
    new MM::Map<MM::Name *, MM::Element *, MM::Name::Compare>();
  //TODO: process maps in recylcer
  return n2e;
}

MM::Map<MM::Name *, MM::Node *, MM::Name::Compare> * MM::Machine::createName2NodeMap()
{
  MM::Map<MM::Name *, MM::Node *, MM::Name::Compare> * n2n =
  new MM::Map<MM::Name *, MM::Node *, MM::Name::Compare>();
  //TODO: process maps in recylcer
  return n2n;
}

MM::String * MM::Machine::createString(MM::UINT32 size)
{
  MM::CHAR * buffer = createBuffer(size+1);
  MM::String * str = new MM::String(buffer,size);
  MM::Recycler::create(str);
  return str;
}

MM::Location * MM::Machine::createLocation(YYLTYPE * loc)
{
  MM::Location * r = new MM::Location(loc->first_line,
                                      loc->first_column,
                                      loc->last_line,
                                      loc->last_column);
  MM::Recycler::create(r);
  return r;
}

MM::Name * MM::Machine::createName(MM::CHAR   * str,
                                   MM::UINT32 * len,
                                   MM::UINT32 * start,
                                   MM::UINT32 * end)
{
  MM::CHAR * buf = createBuffer(*len+1);
  strncpy(buf, str + *start, *len);
  MM::Name * n = new MM::Name(buf, *len);
  MM::Recycler::create(n);  
   *start += *len + 1;
  return n;
}

MM::VOID MM::Machine::eatWhiteSpace(MM::CHAR   * str,
                                    MM::UINT32 * start,
                                    MM::UINT32 * end)
{
  MM::BOOLEAN whitespace = MM_TRUE;
  while(whitespace == MM_TRUE)
  {
    switch(str[*start])
    {
      case ' ':
      case '\t':
      case '\f':
      case '\r':
        (*start)++;
        break;
      default:
        whitespace = MM_FALSE;
        break;
    }
    if(*start >= *end)
    {
      break;
    }
  }
}

MM::Name * MM::Machine::createName(MM::CHAR * str, YYLTYPE  * strLoc)
{
  MM::Name * name = MM_NULL;
  MM::Name * dotRoot = MM_NULL;
  MM::Name * colonRoot = MM_NULL;
  MM::Name * curName = MM_NULL;
  
  MM::UINT32 start = 0;
  MM::UINT32 end = strlen(str);

  do
  {
    MM::UINT32 len1 = strcspn(str + start, ".");
    MM::UINT32 len2 = strcspn(str + start, ":");
    if(len1 <= len2)
    {
      name = createName(str, &len1, &start, &end);
      if(curName != MM_NULL)
      {
        curName->setName(name);
      }
      if(dotRoot == MM_NULL)
      {
        dotRoot = name;
      }
      
      curName = name;
    }
    else
    {
      name = createName(str, &len2, &start, &end);
      if(curName != MM_NULL)
      {
        curName->setName(name);
      }
      if(dotRoot != MM_NULL)
      {
        colonRoot = dotRoot; 
      }
      else
      {
        colonRoot = name;
      }
      dotRoot = MM_NULL;
      curName = MM_NULL;
      eatWhiteSpace(str, &start, &end);
    }
    
  } while(start < end);

  MM::Location * loc = MM::Machine::createLocation(strLoc);
  dotRoot->setLocation(loc);
  dotRoot->setPreName(colonRoot);
  
  return dotRoot;
}

MM::Name * MM::Machine::createName(MM::Name * name)
{
  MM::CHAR * buf = name->getBuffer();
  MM::UINT32 len = name->getLength();
  
  MM::CHAR * buf2 = createBuffer(len+1);
  strncpy(buf2, buf, len);
  
  MM::Name * r = new MM::Name(buf2, len);
  
  MM::Recycler::create(r);
  
  return r;
}

MM::Name * MM::Machine::createName(MM::CHAR * buf)
{
  MM::UINT32 len = strlen(buf);
  MM::CHAR * buf2 = createBuffer(len+1);
  strncpy(buf2, buf, len);
  
  MM::Name * r = new MM::Name(buf2, len);
  
  MM::Recycler::create(r);
  
  return r;
}


MM::Program * MM::Machine::createProgram()
{
  MM::Vector<MM::Transformation *> *
  transformations = createTransformationVector();
  MM::Program * r = new MM::Program(transformations);
  MM::Recycler::create(r);
  return r;  
}

MM::Program * MM::Machine::createProgram(MM::Vector<MM::Transformation *> *
                                         transformations)
{
  MM::Program * r = new MM::Program(transformations);
  MM::Recycler::create(r);
  return r;
}

MM::Modification * MM::Machine::createModification()
{
  MM::Vector<MM::Element *> * elements = createElementVector();
  MM::Modification * r = new MM::Modification(elements);
  MM::Recycler::create(r);
  return r;
}

MM::Modification * MM::Machine::createModification(MM::Vector<MM::Element *> *
                                                   elements)
{
  MM::Modification * r = new MM::Modification(elements);
  MM::Recycler::create(r);
  return r;
}

MM::Modification * MM::Machine::createModification(MM::Vector<MM::Element *> *
                                                   elements,
                                                   YYLTYPE * modifyLoc)
{
  MM::Location * loc = createLocation(modifyLoc);
  MM::Modification * r = new MM::Modification(elements, loc);
  MM::Recycler::create(r);
  return r;
}

MM::Transition * MM::Machine::createTransition()
{
  MM::Vector<MM::Element *> * elements = createElementVector();
  
  MM::Transition * r = new MM::Transition(elements);
  MM::Recycler::create(r);
  return r;
}

MM::Transition * MM::Machine::createTransition(MM::Vector<MM::Element *> *
                                               elements)
{
  MM::Transition * r = new MM::Transition(elements);
  MM::Recycler::create(r);
  return r;
}

MM::Transition * MM::Machine::createTransition(MM::Vector<MM::Element *> *
                                               elements,
                                               YYLTYPE * stepLoc)
{
  MM::Location * loc = createLocation(stepLoc);
  MM::Transition * r = new MM::Transition(elements, loc);
  MM::Recycler::create(r);
  return r;
}

MM::FlowEvent * MM::Machine::createFlowEvent(MM::Instance * actInstance,
                                             MM::Node     * actNode,
                                             MM::Edge     * actEdge,
                                             MM::Instance * srcInstance,
                                             MM::Node     * srcNode,
                                             MM::UINT32     amount,
                                             MM::Instance * tgtInstance,
                                             MM::Node     * tgtNode)
{
  MM::FlowEvent * r = new MM::FlowEvent(actInstance, actNode, actEdge,
                                        srcInstance, srcNode,
                                        amount,
                                        tgtInstance, tgtNode);
  MM::Recycler::create(r);
  return r;
}


MM::TriggerEvent * MM::Machine::createTriggerEvent(MM::Instance * instance,
                                                   MM::Edge     * edge)
{
  MM::TriggerEvent * r = new MM::TriggerEvent(instance, edge);
  MM::Recycler::create(r);
  return r;
}

MM::TriggerEvent * MM::Machine::createTriggerEvent(YYLTYPE * triggerLoc,
                                                   MM::Name * name)
{
  MM::Location * loc = MM::Machine::createLocation(triggerLoc);
  MM::TriggerEvent * r = new MM::TriggerEvent(loc, name);
  MM::Recycler::create(r);
  return r;
}

MM::Prevention * MM::Machine::createPrevention(MM::Instance * instance,
                                               MM::Edge     * edge)
{
  MM::Prevention * r = new MM::Prevention(instance, edge);
  MM::Recycler::create(r);
  return r;
}

MM::Prevention * MM::Machine::createPrevention(YYLTYPE * preventLoc,
                                               MM::Name * name)
{
  MM::Location * loc = MM::Machine::createLocation(preventLoc);
  MM::Prevention * r = new MM::Prevention(loc, name);
  MM::Recycler::create(r);
  return r;
}
	
MM::Failure * MM::Machine::createFailure(MM::Instance * instance,
                                         MM::Node     * node)
{
  MM::Failure * r = new MM::Failure(instance, node);
  MM::Recycler::create(r);
  return r;
}

MM::Failure * MM::Machine::createFailure(YYLTYPE * failLoc,
                                         MM::Name * name)
{
  MM::Location * loc = MM::Machine::createLocation(failLoc);
  MM::Failure * r = new MM::Failure(loc, name);
  MM::Recycler::create(r);
  return r;
}

MM::Activation * MM::Machine::createActivation(MM::Instance * instance,
                                               MM::Node     * node)
{
  MM::Activation * r = new MM::Activation(instance, node);
  MM::Recycler::create(r);
  return r;
}

MM::Activation * MM::Machine::createActivation(YYLTYPE * activateLoc,
                                               MM::Name * name)
{
  MM::Location * loc = MM::Machine::createLocation(activateLoc);
  MM::Activation * r = new MM::Activation(loc, name);
  MM::Recycler::create(r);
  return r; 
}

MM::Enablement * MM::Machine::createEnablement(MM::Instance * instance,
                                               MM::Node     * node)
{
  MM::Enablement * r = new MM::Enablement(instance, node);
  MM::Recycler::create(r);
  return r;
}

MM::Enablement * MM::Machine::createEnablement(YYLTYPE * enableLoc,
                                               MM::Name * name)
{
  MM::Location * loc = MM::Machine::createLocation(enableLoc);
  MM::Enablement * r = new MM::Enablement(loc, name);
  MM::Recycler::create(r);
  return r;
}

MM::Disablement * MM::Machine::createDisablement(MM::Instance * instance,
                                                 MM::Node     * node)
{
  MM::Disablement * r = new MM::Disablement(instance, node);
  MM::Recycler::create(r);
  return r;
}

MM::Disablement * MM::Machine::createDisablement(YYLTYPE * disableLoc,
                                                 MM::Name * name)
{
  MM::Location * loc = MM::Machine::createLocation(disableLoc);
  MM::Disablement * r = new MM::Disablement(loc, name);
  MM::Recycler::create(r);
  return r;
}

MM::Violation * MM::Machine::createViolation(MM::Instance * instance,
                                             MM::Assertion * assertion)
{
  MM::Violation * r = new MM::Violation(instance, assertion);
  MM::Recycler::create(r);
  return r;
}

MM::Violation * MM::Machine::createViolation(YYLTYPE * violateLoc,
                                             MM::Name * name)
{
  MM::Location * loc = MM::Machine::createLocation(violateLoc);
  MM::Violation * r = new MM::Violation(loc, name);
  MM::Recycler::create(r);
  return r;
}

MM::Node * MM::Machine::createSourceNode(MM::NodeBehavior::IO   io,
                                         MM::NodeBehavior::When when,
                                         MM::Name             * name)
{
  MM::SourceNodeBehavior * behavior =
    new MM::SourceNodeBehavior(io,when);
  MM::Recycler::create(behavior);
  MM::Node * r = new MM::Node(name, behavior);
  MM::Recycler::create(r);
  return r;
}

MM::Node * MM::Machine::createDrainNode(MM::NodeBehavior::IO    io,
                                        MM::NodeBehavior::When  when,
                                        MM::NodeBehavior::How   how,
                                        MM::Name      * name)
{
  MM::DrainNodeBehavior * behavior =
    new MM::DrainNodeBehavior(io,when,how);
  MM::Recycler::create(behavior);
  MM::Node * r = new MM::Node(name,behavior);
  MM::Recycler::create(r);
  return r;
}

MM::Node * MM::Machine::createGateNode(MM::NodeBehavior::IO    io,
                                       MM::NodeBehavior::When  when,
                                       MM::NodeBehavior::Act   act,
                                       MM::NodeBehavior::How   how,
                                       MM::Name      * name)
{
  MM::GateNodeBehavior * behavior =
    new MM::GateNodeBehavior(io,when,act,how);
  MM::Recycler::create(behavior);
  
  MM::Node * r = new MM::Node(name, behavior);
  MM::Recycler::create(r);
  return r;
}

MM::Node * MM::Machine::createPoolNode(MM::NodeBehavior::IO    io,
                                       MM::NodeBehavior::When  when,
                                       MM::NodeBehavior::Act   act,
                                       MM::NodeBehavior::How   how,
                                       MM::Name      * name,
                                       MM::Name      * of,
                                       MM::UINT32      at,
                                       MM::UINT32      max,
                                       MM::Exp       * exp)
{
  MM::Map<MM::Name *, MM::Node *, MM::Name::Compare> * interfaces =
  createName2NodeMap();
  
  MM::PoolNodeBehavior * behavior =
    new MM::PoolNodeBehavior(io,when,act,how,of,at,max,exp,interfaces);
  MM::Recycler::create(behavior);
  
  MM::Node * r = new MM::Node(name, behavior);
  MM::Recycler::create(r);
  return r;
}

MM::Node * MM::Machine::createConverterNode(MM::NodeBehavior::IO    io,
                                            MM::NodeBehavior::When  when,
                                            MM::Name      * name,
                                            MM::Name      * from,
                                            MM::Name      * to)
{
  MM::ConverterNodeBehavior * behavior =
    new MM::ConverterNodeBehavior(io, when, from, to);
  MM::Recycler::create(behavior);
  
  MM::Node * r = new MM::Node(name, behavior);
  //converter does not own edges
  r->setEdgeOwnership(MM_FALSE);
  MM::Recycler::create(r);
  return r;
}

MM::Node * MM::Machine::createRefNode(MM::NodeBehavior::IO io, MM::Name * name)
{
  MM::RefNodeBehavior * behavior = new MM::RefNodeBehavior(io);
  MM::Recycler::create(behavior);
  MM::Node * r = new MM::Node(name, behavior);
  MM::Recycler::create(r);
  return r;
}


MM::InterfaceNode * MM::Machine::createInterfaceNode(MM::Name * name,
                                                     MM::Element * parent,
                                                     MM::Node * ref)
{
  MM::InterfaceNode * r = new MM::InterfaceNode(name, parent, ref);
  MM::Recycler::create(r);
  return r;
}

MM::StateEdge * MM::Machine::createStateEdge(MM::Name * name,
                                             MM::Name * src,
                                             MM::Exp  * exp,
                                             MM::Name * tgt)
{
  MM::StateEdge * r = new MM::StateEdge(name,src,exp,tgt);
  MM::Recycler::create(r);
  return r;
}

MM::StateEdge * MM::Machine::createAnonymousTriggerEdge(MM::Node * src,
                                                        MM::Node * tgt)
{
  MM::Exp * e1 = createOneExp();
  MM::Exp * e2 = createOneExp();
  MM::Exp * exp = createBinExp(e1, MM::Operator::OP_MUL, e2);
  
  MM::Name * srcName = src->getName();
  MM::Name * tgtName = tgt->getName();
  
  MM::Name * triggerSrcName = createName(srcName);
  MM::Name * triggerTgtName = createName(tgtName);
  
  MM::StateEdge * triggerEdge = createStateEdge(MM_NULL, triggerSrcName, exp, triggerTgtName);
  triggerEdge->setSource(src);
  triggerEdge->setTarget(tgt);
  return triggerEdge;
}

MM::FlowEdge * MM::Machine::createFlowEdge(MM::Name * name,
                                           MM::Name * src,
                                           MM::Exp  * exp,
                                           MM::Name * tgt)
{
  MM::FlowEdge * r = new MM::FlowEdge(name,src,exp,tgt);
  MM::Recycler::create(r);
  return r;
}

MM::Definition * MM::Machine::createDefinition()
{
  MM::Vector<Element*> * elements = createElementVector();  
  MM::Definition * r = new MM::Definition(MM_NULL, elements);
  MM::Recycler::create(r);
  return r;
}

MM::Definition * MM::Machine::createDefinition(MM::Name * name,
                                               MM::Vector<Element*> * elements)
{
  MM::Definition * r = new MM::Definition(name, elements);
  MM::Recycler::create(r);
  return r;
}


MM::Declaration * MM::Machine::createDeclaration(MM::Name * type,
                                                 MM::Name * name)
{
  MM::Map<MM::Name *, MM::Node *, MM::Name::Compare> * interfaces =
    createName2NodeMap();
  MM::Declaration * r = new MM::Declaration(type, name, interfaces);
  MM::Recycler::create(r);
  return r;
}

MM::Assertion * MM::Machine::createAssertion(YYLTYPE  * assertLoc,
                                             MM::Name * name,
                                             MM::Exp  * exp,
                                             MM::CHAR * msg)
{
  MM::Location * loc = MM::Machine::createLocation(assertLoc);
  MM::UINT32 len = strlen(msg);

  MM::CHAR * buf = MM::Recycler::createBuffer(len+1);
  strncpy(buf, msg, len);
  MM::Assertion * r = new MM::Assertion(name,exp,buf,loc);
  
  MM::Recycler::create(r);
  return r;
}

MM::Assertion * MM::Machine::createAssertion(MM::Name * name,
                                             MM::Exp  * exp,
                                             MM::CHAR * msg)
{
  MM::UINT32 len = strlen(msg);
  MM::CHAR * buf = MM::Recycler::createBuffer(len+1);
  strncpy(buf, msg, len);
  MM::Assertion * r = new MM::Assertion(name,exp,buf);
  
  MM::Recycler::create(r);
  return r;
}

MM::Deletion * MM::Machine::createDeletion(YYLTYPE * deleteLoc,
                                           MM::Name * name)
{
  MM::Location * loc = MM::Machine::createLocation(deleteLoc);
  MM::Deletion * r = new MM::Deletion(loc, name);
  MM::Recycler::create(r);
  return r;
}

MM::Deletion * MM::Machine::createDeletion(MM::Name * name)
{
  MM::Deletion * r = new MM::Deletion(name);
  MM::Recycler::create(r);
  return r;
}

MM::UnExp * MM::Machine::createUnExp(MM::Operator::OP  op,
                                     YYLTYPE         * opLoc,
                                     MM::Exp         * exp)
{
  MM::Location * loc = MM::Machine::createLocation(opLoc);
  MM::UnExp * r = new MM::UnExp(op, exp, loc);
  MM::Recycler::create(r);
  return r;
}

MM::UnExp * MM::Machine::createUnExp(MM::Operator::OP  op,
                                     MM::Exp         * exp)
{
  MM::UnExp * r = new MM::UnExp(op, exp);
  MM::Recycler::create(r);
  return r;
}

MM::BinExp * MM::Machine::createBinExp(MM::Exp          * e1,
                                       MM::Operator::OP   op,
                                       YYLTYPE          * opLoc,
                                       MM::Exp          * e2)
{
  MM::Location * loc = MM::Machine::createLocation(opLoc);
  MM::BinExp * r = new MM::BinExp(e1,op,e2,loc);
  MM::Recycler::create(r);
  return r;
}


MM::BinExp * MM::Machine::createBinExp(MM::Exp          * e1,
                                       MM::Operator::OP   op,
                                       MM::Exp          * e2)
{
  MM::BinExp * r = new MM::BinExp(e1,op,e2);
  MM::Recycler::create(r);
  return r;
}

MM::OverrideExp * MM::Machine::createOverrideExp(MM::Exp * e)
{
  MM::OverrideExp * r = new MM::OverrideExp(e);
  MM::Recycler::create(r);
  return r;
}

MM::OverrideExp * MM::Machine::createOverrideExp(YYLTYPE * lparenLoc,
                                                 MM::Exp * e,
                                                 YYLTYPE * rparenLoc)
{
  MM::Location * loc1 = createLocation(lparenLoc);
  MM::Location * loc2 = createLocation(rparenLoc);
  MM::OverrideExp * r = new MM::OverrideExp(loc1, e, loc2);
  MM::Recycler::create(r);
  return r;
}

MM::RangeValExp * MM::Machine::createRangeValExp(MM::INT32   v1,
                                                 YYLTYPE   * v1Loc,
                                                 YYLTYPE   * rangeLoc,
                                                 MM::INT32   v2,
                                                 YYLTYPE   * v2Loc)
{
  MM::Location * v1_loc = MM::Machine::createLocation(v1Loc);
  MM::Location * v2_loc = MM::Machine::createLocation(v2Loc);
  MM::Location * range_loc = MM::Machine::createLocation(rangeLoc);
  MM::RangeValExp * r = new MM::RangeValExp(v1, v2, v1_loc, range_loc, v2_loc);
  MM::Recycler::create(r);
  return r;
}

MM::RangeValExp * MM::Machine::createRangeValExp(MM::INT32   v1,
                                                 MM::INT32   v2)
{
  MM::RangeValExp * r = new MM::RangeValExp(v1, v2);
  MM::Recycler::create(r);
  return r;
}

MM::NumberValExp * MM::Machine::createNumberValExp(MM::INT32  val,
                                                   YYLTYPE  * valLoc)
{
  MM::Location * loc = MM::Machine::createLocation(valLoc);  
  MM::NumberValExp * r = new MM::NumberValExp(val/100, val%100, loc);
  MM::Recycler::create(r);
  return r;
}

MM::NumberValExp * MM::Machine::createNumberValExp(MM::INT32 val)
{
  MM::NumberValExp * r = new MM::NumberValExp(val/100, val%100);
  MM::Recycler::create(r);
  return r;
}


MM::BooleanValExp * MM::Machine::createBooleanValExp(MM::BOOLEAN val,
                                                     YYLTYPE * valLoc)
{
  MM::Location * loc = MM::Machine::createLocation(valLoc);
  MM::BooleanValExp * r = new MM::BooleanValExp(val, loc);
  MM::Recycler::create(r);
  return r;
};

MM::BooleanValExp * MM::Machine::createBooleanValExp(MM::BOOLEAN val)
{
  MM::BooleanValExp * r = new MM::BooleanValExp(val, MM_NULL);
  MM::Recycler::create(r);
  return r;
};

MM::AllExp * MM::Machine::createAllExp(YYLTYPE * allLoc)
{
  MM::Location * loc = MM::Machine::createLocation(allLoc);
  MM::AllExp * r = new MM::AllExp(loc);
  MM::Recycler::create(r);
  return r;
};

MM::ActiveExp * MM::Machine::createActiveExp(YYLTYPE  * activeLoc,
                                             MM::Name * name)
{
  MM::Location * loc = MM::Machine::createLocation(activeLoc);
  MM::ActiveExp * r = new MM::ActiveExp(name,loc);
  MM::Recycler::create(r);
  return r;
}

MM::AliasExp * MM::Machine::createAliasExp(YYLTYPE * aliasLoc)
{
  MM::Location * loc = createLocation(aliasLoc);
  MM::AliasExp * r = new MM::AliasExp(loc);
  MM::Recycler::create(r);
  return r;
}

MM::OneExp * MM::Machine::createOneExp()
{
  MM::OneExp * r = new MM::OneExp(MM_NULL);
  MM::Recycler::create(r);
  return r;
}

MM::OneExp * MM::Machine::createOneExp(YYLTYPE * epsilonLoc)
{
  MM::Location * loc = createLocation(epsilonLoc);
  MM::OneExp * r = new MM::OneExp(loc);
  MM::Recycler::create(r);
  return r;
}

MM::VarExp * MM::Machine::createVarExp(MM::Name * name)
{
  MM::VarExp * r = new MM::VarExp(name);
  MM::Recycler::create(r);
  return r;
}

MM::Instance * MM::Machine::createInstance(MM::Instance * parent,
                                           MM::Definition * def,
                                           MM::Element * decl)
{
  MM::Instance * instance = new MM::Instance(parent, def, decl);
  //MM::Node * node = MM_NULL;
  //MM::NodeBehavior * behavior = MM_NULL;
  //MM::Vector<Element *> * elements = def->getElements();
  //MM::Vector<Element *>::Iterator eIter = elements->getIterator();
  //MM::PoolNodeBehavior * poolNodeBehavior = MM_NULL;
  //MM::UINT32 at = 0;
  //MM::Exp * exp = MM_NULL;
  //MM::PoolNodeInstance * poolNodeInstance = MM_NULL;

  /*
  //create poolNodeInstances
  while(eIter.hasNext() == MM_TRUE)
  {
    MM::Element * element = eIter.getNext();
    switch(element->getTypeId())
    {
     case MM::T_Node:
       node = (MM::Node*) element;
       behavior = node->getBehavior();

       if(behavior->instanceof(MM::T_PoolNodeBehavior) == MM_TRUE)
       {
         poolNodeBehavior = (MM::PoolNodeBehavior *) behavior;
         at = poolNodeBehavior->getAt();
         poolNodeInstance = new PoolNodeInstance(node, instance, at);
         instance->addPoolNodeInstance(poolNodeInstance);
       }
       break;
     default:
       break;
    }
  }

  //initialize poolNodeInstances (can only be done after creation!)
  eIter.reset();
  while(eIter.hasNext() == MM_TRUE)
  {
    MM::Element * element = eIter.getNext();
    switch(element->getTypeId())
    {
     case MM::T_Node:
       node = (MM::Node*) element;
       behavior = node->getBehavior();

       if(behavior->instanceof(MM::T_PoolNodeBehavior) == MM_TRUE)
       {
         poolNodeBehavior = (MM::PoolNodeBehavior *) behavior;
         exp = poolNodeBehavior->getAdd();
         poolNodeInstance = instance->getPoolNodeInstance(node);
         if(exp != MM_NULL)
         {
           poolNodeInstance->initExp(exp);
         }
       }
       break;
     default:
       break;
    }
  }
  */

  MM::Recycler::create(instance);
  return instance;
}