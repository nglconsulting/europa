/**
 * @file Main.cc
 *
 * @brief Provides an executable for your project which uses
 * - a standard chronological backtracking planner
 * - a PSEngine to encapsulate EUROPA
 */

#include "Nddl.hh" /*!< Includes protypes required to load a model */
#include "PSEngine.hh"
#include "Debug.hh"

using namespace EUROPA;

bool solve(bool useInterpreter, const char* plannerConfig, const char* txSource, int startHorizon, int endHorizon, int maxSteps);
void runSolver(PSSolver* solver, int startHorizon, int endHorizon, int maxSteps);
void checkSolver(PSSolver* solver, int i);
void printFlaws(int it, PSList<std::string>& flaws);

int main(int argc, const char ** argv)
{
  if (argc < 3) {
    std::cerr << "Must provide initial transactions file." << std::endl;
    return -1;
  }

  const char* txSource = argv[1];
  const char* plannerConfig = argv[2];
  bool useInterpreter = (argc > 3);

  solve(
      useInterpreter,
      plannerConfig,
      txSource,
      0,   // startHorizon
      100, // endHorizon
      1000 // maxSteps
  );

  return 0;
}

bool solve(bool useInterpreter,
           const char* plannerConfig,
           const char* txSource,
           int startHorizon,
           int endHorizon,
           int maxSteps)
{
    try {

      {
	      PSEngine* engine = PSEngine::makeInstance();
          engine->start();

          if (!useInterpreter) {
              SchemaId schema = ((Schema*)assembly.getComponent("Schema"))->getId();
              RuleSchemaId ruleSchema = ((RuleSchema*)assembly.getComponent("RuleSchema"))->getId();
              NDDL::loadSchema(schema,ruleSchema); // eventually make this called via dlopen
              engine->executeScript("nddl-xml-txn",txSource,true/*isFile*/);
          }
          else
	          engine->executeScript("nddl-xml",txSource,true/*isFile*/);

	      PSSolver* solver = engine->createSolver(plannerConfig);
	      runSolver(solver,startHorizon,endHorizon,maxSteps);
	      delete solver;

	      delete engine;
      }

	  return true;
	}
	catch (Error& e) {
		std::cerr << "PSEngine failed:" << e.getMsg() << std::endl;
		return false;
	}
}

void printFlaws(int it, PSList<std::string>& flaws)
{
	debugMsg("Main","Iteration:" << it << " " << flaws.size() << " flaws");

	for (int i=0; i<flaws.size(); i++) {
		debugMsg("Main", "    " << (i+1) << " - " << flaws.get(i));
	}
}

void runSolver(PSSolver* solver, int startHorizon, int endHorizon, int maxSteps)
{
    solver->configure(startHorizon,endHorizon);
    int i;
    for (i = 0;
         !solver->isExhausted() &&
         !solver->isTimedOut() &&
         i<maxSteps;
         i = solver->getStepCount()) {

  	  solver->step();
  	  PSList<std::string> flaws;
  	  if (solver->isConstraintConsistent()) {
  		  flaws = solver->getFlaws();
  		  printFlaws(i,flaws);
  		  if (flaws.size() == 0)
  			  break;
  	  }
  	  else
  		  debugMsg("Main","Iteration " << i << " Solver is not constraint consistent");
    }

    checkSolver(solver,i);
}

void checkSolver(PSSolver* solver, int i)
{
    if (solver->isExhausted()) {
  	  debugMsg("Main","Solver was exhausted after " << i << " steps");
    }
    else if (solver->isTimedOut()) {
  	  debugMsg("Main","Solver timed out after " << i << " steps");
    }
    else {
  	  debugMsg("Main","Solver finished after " << i << " steps");
    }
}

