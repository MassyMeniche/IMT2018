#include "binomialtree.hpp"
#include "binomialengine.hpp"
#include <ql/methods/lattices/tree.hpp>

// Added dependencies for Option creation
#include <ql/qldefines.hpp>
#ifdef BOOST_MSVC
#  include <ql/auto_link.hpp>
#endif
#include <ql/instruments/vanillaoption.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/utilities/dataformatters.hpp>

#include <ql/pricingengines/vanilla/analyticeuropeanengine.hpp>

#include <boost/timer.hpp>
#include <iomanip>
#include <iostream>

#include <chrono>
#include <ctime>

using namespace QuantLib;


int main() {

    try {
 		  //  Option caracteristics
 		  boost::timer timer;
        std::cout << std::endl;

        // set up dates
        Calendar calendar = TARGET();
        Date todaysDate(15, Jan, 2019);
        Date settlementDate(17, Jan, 2019);
        Settings::instance().evaluationDate() = todaysDate;

        // set up option characteristics
        Option::Type type(Option::Put);
        Real underlying = 334;
        Real strike = 300;
        Spread dividendYield = 0.00;
        Rate riskFreeRate = 0.001;
        Volatility volatility = 0.20;
        Date maturity(17, Jan, 2020);
        DayCounter dayCounter = Actual365Fixed();

        //Retrieve option characteristics
        std::cout << "Option type = "  << type << std::endl;
        std::cout << "Maturity = "        << maturity << std::endl;
        std::cout << "Underlying price = "        << underlying << std::endl;
        std::cout << "Strike = "                  << strike << std::endl;
        std::cout << "Risk-free interest rate = " << io::rate(riskFreeRate)
                  << std::endl;
        std::cout << "Dividend yield = " << io::rate(dividendYield)
                  << std::endl;
        std::cout << "Volatility = " << io::volatility(volatility)
                  << std::endl;
        std::cout << std::endl;
        std::string method;

        //Set up exercise
        boost::shared_ptr<Exercise> europeanExercise(new EuropeanExercise(maturity));


        // Market Data
        Handle<Quote> underlyingH(boost::shared_ptr<Quote>(new SimpleQuote(underlying)));

        //Bootstrap the yield/dividend/vol curves

        Handle<YieldTermStructure> flatTermStructure(boost::shared_ptr<YieldTermStructure>(new FlatForward(settlementDate, riskFreeRate, dayCounter)));

        Handle<YieldTermStructure> flatDividendTS(boost::shared_ptr<YieldTermStructure>(new FlatForward(settlementDate, dividendYield, dayCounter)));

        Handle<BlackVolTermStructure> flatVolTS(boost::shared_ptr<BlackVolTermStructure>(new BlackConstantVol(settlementDate, calendar, volatility,dayCounter)));


        // Pay off construction
        boost::shared_ptr<StrikedTypePayoff> payoff(new PlainVanillaPayoff(type, strike));

        boost::shared_ptr<BlackScholesMertonProcess> bsmProcess(new BlackScholesMertonProcess(underlyingH, flatDividendTS,flatTermStructure, flatVolTS));

        // Option Definition
        VanillaOption europeanOption(payoff, europeanExercise);


        // Greeks calculated using the analytic B&S formula

        europeanOption.setPricingEngine(boost::shared_ptr<PricingEngine>(
                                    new AnalyticEuropeanEngine(bsmProcess)));
        std::cout <<  std::endl;

        auto starttimeBSDelta = std::chrono::system_clock::now();

        std::cout << "Black & Scholes Delta: "<<europeanOption.delta() << std::endl;

        auto endtimeBSDelta = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_secondsBSDelta = endtimeBSDelta-starttimeBSDelta;
        std::time_t end_timeBSDelta = std::chrono::system_clock::to_time_t(endtimeBSDelta);
        std::cout << "Black&Scholes Delta calculation time: " << elapsed_secondsBSDelta.count() << "s\n";

        auto starttimeBSGamma = std::chrono::system_clock::now();

        std::cout << "Black & Scholes Gamma: "<<europeanOption.gamma() << std::endl;

        auto endtimeBSGamma = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_secondsBSGamma = endtimeBSGamma-starttimeBSGamma;
        std::time_t end_timeBSGamma = std::chrono::system_clock::to_time_t(endtimeBSGamma);
        std::cout << "Black&Scholes Gamma calculation time: " << elapsed_secondsBSGamma.count() << "s\n";
        std::cout << std::endl ;
        std::cout << std::endl ;



        Size timeSteps = 801;

        // Pricing Engine : BinomialVanillaEngine<CoxRossRubinstein>
        std::cout << "Cox Ross Rubinstein: "<<std::endl;

        europeanOption.setPricingEngine(boost::shared_ptr<PricingEngine>(new BinomialVanillaEngine_2<CoxRossRubinstein_2>(bsmProcess,timeSteps)));
        //std::cout << europeanOption.NPV() << std::endl;

        // Greeks calculated using the binomial tree (New implementation)

        auto starttimeTreeDeltaC = std::chrono::system_clock::now();

        std::cout << "Delta calculated with the New Binomial engine: "<<europeanOption.delta() << std::endl;

        auto endtimeTreeDeltaC = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_secondsTreeDeltaC = endtimeTreeDeltaC-starttimeTreeDeltaC;
        std::time_t end_timeTreeDeltaC = std::chrono::system_clock::to_time_t(endtimeTreeDeltaC);
        std::cout << "Binomial Tree Delta calculation time: " << elapsed_secondsTreeDeltaC.count() << "s\n";

        auto starttimeTreeGammaC = std::chrono::system_clock::now();

        std::cout << "Gamma calculated with the New Binomial engine: "<<europeanOption.gamma() << std::endl;

        auto endtimeTreeGammaC = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_secondsTreeGammaC = endtimeTreeGammaC-starttimeTreeGammaC;
        std::time_t end_timeTreeGammaC = std::chrono::system_clock::to_time_t(endtimeTreeGammaC);
        std::cout << "Binomial Tree Gamma calculation time: " << elapsed_secondsTreeGammaC.count() << "s\n";
        std::cout << std::endl ;

        // Pricing Engine : BinomialVanillaEngine<Trigeorgis>

        std::cout << "Trigeorgis: " << std::endl;

        europeanOption.setPricingEngine(boost::shared_ptr<PricingEngine>(new BinomialVanillaEngine_2<Trigeorgis_2>(bsmProcess,timeSteps)));
        //std::cout << europeanOption.NPV() << std::endl;

        // Greeks calculated using the binomial tree (new implementation)

        auto starttimeTreeDeltaT = std::chrono::system_clock::now();

        std::cout << "Delta calculated with the New Binomial engine: "<<europeanOption.delta() << std::endl;

        auto endtimeTreeDeltaT = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_secondsTreeDeltaT = endtimeTreeDeltaT-starttimeTreeDeltaT;
        std::time_t end_timeTreeDeltaT = std::chrono::system_clock::to_time_t(endtimeTreeDeltaT);
        std::cout << "Binomial Tree Delta calculation time: " << elapsed_secondsTreeDeltaT.count() << "s\n";

        auto starttimeTreeGammaT = std::chrono::system_clock::now();

        std::cout << "Gamma calculated with the New Binomial engine: "<<europeanOption.gamma() << std::endl;

        auto endtimeTreeGammaT = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_secondsTreeGammaT = endtimeTreeGammaT-starttimeTreeGammaT;
        std::time_t end_timeTreeGammaT = std::chrono::system_clock::to_time_t(endtimeTreeGammaT);
        std::cout << "Binomial Tree Gamma calculation time: " << elapsed_secondsTreeGammaT.count() << "s\n";
        std::cout << std::endl ;

         // Pricing Engine : BinomialVanillaEngine<JarrowRudd>

        std::cout << "Jarrow Rudd: " << std::endl;

        europeanOption.setPricingEngine(boost::shared_ptr<PricingEngine>(new BinomialVanillaEngine_2<JarrowRudd_2>(bsmProcess,timeSteps)));
        //std::cout << europeanOption.NPV() << std::endl;

        // Greeks calculated using the binomial tree (New implementation)

        auto starttimeTreeDeltaJ = std::chrono::system_clock::now();

        std::cout << "Delta calculated with the New Binomial engine: "<<europeanOption.delta() << std::endl;

        auto endtimeTreeDeltaJ = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_secondsTreeDeltaJ = endtimeTreeDeltaJ-starttimeTreeDeltaJ;
        std::time_t end_timeTreeDeltaJ = std::chrono::system_clock::to_time_t(endtimeTreeDeltaJ);
        std::cout << "Binomial Tree Delta calculation time: " << elapsed_secondsTreeDeltaJ.count() << "s\n";

        auto starttimeTreeGammaJ = std::chrono::system_clock::now();

        std::cout << "Gamma calculated with the New Binomial engine: "<<europeanOption.gamma() << std::endl;

        auto endtimeTreeGammaJ = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_secondsTreeGammaJ = endtimeTreeGammaJ-starttimeTreeGammaJ;
        std::time_t end_timeTreeGammaJ = std::chrono::system_clock::to_time_t(endtimeTreeGammaJ);
        std::cout << "Binomial Tree Gamma calculation time: " << elapsed_secondsTreeGammaJ.count() << "s\n";
        std::cout << std::endl ;

        // Pricing Engine : BinomialVanillaEngine<AdditiveEQPBinomialTree>

        std::cout << "Additive EQP Binomial Tree: " << std::endl;

        europeanOption.setPricingEngine(boost::shared_ptr<PricingEngine>(new BinomialVanillaEngine_2<AdditiveEQPBinomialTree_2>(bsmProcess,timeSteps)));
        //std::cout << europeanOption.NPV() << std::endl;

        // Greeks calculated using the binomial tree (New implementation)

        auto starttimeTreeDeltaA = std::chrono::system_clock::now();

        std::cout << "Delta calculated with the New Binomial engine: "<<europeanOption.delta() << std::endl;

        auto endtimeTreeDeltaA = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_secondsTreeDeltaA = endtimeTreeDeltaA-starttimeTreeDeltaA;
        std::time_t end_timeTreeDeltaA = std::chrono::system_clock::to_time_t(endtimeTreeDeltaA);
        std::cout << "Binomial Tree Delta calculation time: " << elapsed_secondsTreeDeltaA.count() << "s\n";

        auto starttimeTreeGammaA = std::chrono::system_clock::now();

        std::cout << "Gamma calculated with the New Binomial engine: "<<europeanOption.gamma() << std::endl;

        auto endtimeTreeGammaA = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_secondsTreeGammaA = endtimeTreeGammaA-starttimeTreeGammaA;
        std::time_t end_timeTreeGammaA = std::chrono::system_clock::to_time_t(endtimeTreeGammaA);
        std::cout << "Binomial Tree Gamma calculation time: " << elapsed_secondsTreeGammaA.count() << "s\n";
        std::cout << std::endl ;

        // Pricing Engine : BinomialVanillaEngine<Tian>

        std::cout << "Tian: " << std::endl;

        europeanOption.setPricingEngine(boost::shared_ptr<PricingEngine>(new BinomialVanillaEngine_2<Tian_2>(bsmProcess,timeSteps)));
        //std::cout << europeanOption.NPV() << std::endl;

        // Greeks calculated using the binomial tree (New implementation)

        auto starttimeTreeDeltaTI = std::chrono::system_clock::now();

        std::cout << "Delta calculated with the New Binomial engine: "<<europeanOption.delta() << std::endl;

        auto endtimeTreeDeltaTI = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_secondsTreeDeltaTI = endtimeTreeDeltaTI-starttimeTreeDeltaTI;
        std::time_t end_timeTreeDeltaTI = std::chrono::system_clock::to_time_t(endtimeTreeDeltaTI);
        std::cout << "Binomial Tree Delta calculation time: " << elapsed_secondsTreeDeltaTI.count() << "s\n";

        auto starttimeTreeGammaTI = std::chrono::system_clock::now();

        std::cout << "Gamma calculated with the New Binomial engine: "<<europeanOption.gamma() << std::endl;

        auto endtimeTreeGammaTI = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_secondsTreeGammaTI = endtimeTreeGammaTI-starttimeTreeGammaTI;
        std::time_t end_timeTreeGammaTI = std::chrono::system_clock::to_time_t(endtimeTreeGammaTI);
        std::cout << "Binomial Tree Gamma calculation time: " << elapsed_secondsTreeGammaTI.count() << "s\n";
        std::cout << std::endl ;


        // Pricing Engine : BinomialVanillaEngine<LeisenReimer>

        std::cout << "Leisen Reimer: " << std::endl;

        europeanOption.setPricingEngine(boost::shared_ptr<PricingEngine>(new BinomialVanillaEngine_2<LeisenReimer_2>(bsmProcess,timeSteps)));
        //std::cout << europeanOption.NPV() << std::endl;

        // Greeks calculated using the binomial tree (New implementation)

        auto starttimeTreeDeltaL = std::chrono::system_clock::now();

        std::cout << "Delta calculated with the New Binomial engine: "<<europeanOption.delta() << std::endl;

        auto endtimeTreeDeltaL = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_secondsTreeDeltaL = endtimeTreeDeltaL-starttimeTreeDeltaL;
        std::time_t end_timeTreeDeltaL = std::chrono::system_clock::to_time_t(endtimeTreeDeltaL);
        std::cout << "Binomial Tree Delta calculation time: " << elapsed_secondsTreeDeltaL.count() << "s\n";

        auto starttimeTreeGammaL = std::chrono::system_clock::now();

        std::cout << "Gamma calculated with the New Binomial engine: "<<europeanOption.gamma() << std::endl;

        auto endtimeTreeGammaL = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_secondsTreeGammaL = endtimeTreeGammaL-starttimeTreeGammaL;
        std::time_t end_timeTreeGammaL = std::chrono::system_clock::to_time_t(endtimeTreeGammaL);
        std::cout << "Binomial Tree Gamma calculation time: " << elapsed_secondsTreeGammaL.count() << "s\n";
        std::cout << std::endl ;


        // Pricing Engine : BinomialVanillaEngine<Joshi4>

        std::cout << "Joshi: " << std::endl;

        europeanOption.setPricingEngine(boost::shared_ptr<PricingEngine>(new BinomialVanillaEngine_2<Joshi4_2>(bsmProcess,timeSteps)));
        //std::cout << europeanOption.NPV() << std::endl;

        // Greeks calculated using the binomial tree (New implementation)

        auto starttimeTreeDeltaJO = std::chrono::system_clock::now();

        std::cout << "Delta calculated with the New Binomial engine: "<<europeanOption.delta() << std::endl;

        auto endtimeTreeDeltaJO = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_secondsTreeDeltaJO = endtimeTreeDeltaJO-starttimeTreeDeltaJO;
        std::time_t end_timeTreeDeltaJO = std::chrono::system_clock::to_time_t(endtimeTreeDeltaJO);
        std::cout << "Binomial Tree Delta calculation time: " << elapsed_secondsTreeDeltaJO.count() << "s\n";

        auto starttimeTreeGammaJO = std::chrono::system_clock::now();

        std::cout << "Gamma calculated with the New Binomial engine: "<<europeanOption.gamma() << std::endl;

        auto endtimeTreeGammaJO = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_secondsTreeGammaJO = endtimeTreeGammaJO-starttimeTreeGammaJO;
        std::time_t end_timeTreeGammaJO = std::chrono::system_clock::to_time_t(endtimeTreeGammaJO);
        std::cout << "Binomial Tree Gamma calculation time: " << elapsed_secondsTreeGammaJO.count() << "s\n";
        std::cout << std::endl ;

        return 0;

    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "unknown error" << std::endl;
        return 1;
    }
}
