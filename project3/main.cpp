
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
        std::cout << std::endl ;

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

        // Pricing Engine
        Size timeSteps = 801;
        europeanOption.setPricingEngine(boost::shared_ptr<PricingEngine>(new BinomialVanillaEngine_2<CoxRossRubinstein_2>(bsmProcess,timeSteps)));
        //std::cout << europeanOption.NPV() << std::endl;

        // Greeks calculated using the binomial tree
        std::cout << "Delta calculated with the actual Binomial engine: "<<europeanOption.delta() << std::endl;
        std::cout << "Gamma calculated with the actual Binomial engine: "<<europeanOption.gamma() << std::endl;

        // Greeks calculated using the analytic B&S formula
        method = "Black-Scholes";
        europeanOption.setPricingEngine(boost::shared_ptr<PricingEngine>(
                                    new AnalyticEuropeanEngine(bsmProcess)));
        std::cout <<  std::endl;
        std::cout << "Black & Scholes Delta: "<<europeanOption.delta() << std::endl;
        std::cout << "Black & Scholes Gamma: "<<europeanOption.gamma() << std::endl;

        return 0;

    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "unknown error" << std::endl;
        return 1;
    }
}

