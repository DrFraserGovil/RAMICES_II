#pragma once

#include "../../settings/SimulationSettings.h"
#include "../../utility/Log.h"
#include <vector>

enum class Move {Fraction, Mass};

class Gas
{
    public:
	
        // Factory functions (no public constructor)

        //! \brief Creates an empty gas object (zero mass, zero composition).
        //! \return An empty Gas object.
        static Gas Empty();

        //! \brief Creates a gas with specified total mass and elemental composition.
        //! \param mass Total mass of the gas.
        //! \param composition Fractional elemental composition (vector of size ::Element::Count).
        //! \return A Gas object with the given mass and composition.
        //! \throws std::logic_error If mass is negative.
        //! \throws std::runtime_error If composition vector size is incorrect.
        //! \throws Warning If mass is zero.
        static Gas WithComposition(double mass, const std::vector<double> & composition,bool forceAcceptZeroMass = false);

        //! \brief Creates a new gas with specified mass and the same composition as a target gas.
        //! \param mass Desired total mass for the new gas.
        //! \param targetGas The Gas object whose composition will be copied.
        //! \return A new Gas object.
        //! \throws std::logic_error If targetGas has zero mass.
        static Gas WithSameComposition(double mass, const Gas & targetGas);

		 //! \brief Creates a new gas with the same composition as a target gas, and amass equal to a specified fraction of the original mass.
        //! \param fraction The value used to compute new_mass = fraction * targetGas.Mass() for the new object 
        //! \param targetGas The Gas object whose composition will be copied.
        //! \return A new Gas object.
        //! \throws std::logic_error If targetGas has zero mass.
		static Gas FractionalCopy(double fraction, const Gas & target);

        //! \brief Creates a gas object from an elemental mass grid.
        //! \param massArray Vector of absolute masses for each element (size `Element::Count`).
        //! \return A Gas object with the specified elemental mass distribution.
        //! \throws std::runtime_error If massArray size is incorrect.
        //! \throws std::logic_error If any element in massArray is negative.
        static Gas WithMass(const std::vector<double> & massArray);

        //! \brief Creates a gas object with specified mass and primordial elemental abundances.
        //! \param mass Total mass of the primordial gas.
        //! \return A primordial Gas object.
        static Gas Primordial(double mass);

        // Property queries (const methods)

        //! \brief Returns a const reference to the elemental composition (mass fractions).
        //! \return `std::vector<double>` of elemental mass fractions.
        const std::vector<double> & Composition() const;

        //! \brief Returns the total mass of the gas.
        //! \return Total mass.
        double Mass() const;

        //! \brief Returns the metallicity (mass fraction of elements heavier than H/He).
        //! \return Metallicity (0.0 to 1.0).
        double Metallicity() const;

        //! \brief Returns the mass fraction of a specific elemental species.
        //! \param id The `Element::Species`
        //! \return Fractional abundance of the element.
        double FractionOf(Element::Species id) const;

        //! \brief Returns the absolute mass of a specific elemental species.
        //! \param id The `Element::Species` 
        //! \return Absolute mass of the element.
        double MassOf(Element::Species id) const;

        // Operators (Access and Modification)

        //! \brief Mutable access to an element's mass by `Element::Species` ID.
        //! \param id `Element::Species` ID.
        //! \return Mutable reference to the element's mass.
        double & operator[](Element::Species id);

        //! \brief Mutable access to an element's mass by integer index.
        //! \param id Integer index.
        //! \return Mutable reference to the element's mass.
        double & operator[](int id);

        //! \brief Read-only access to an element's mass by `Element::Species` ID.
        //! \param id `Element::Species` ID.
        //! \return Constant mass of the element.
        double  operator[](Element::Species id) const;

        //! \brief Read-only access to an element's mass by integer index.
        //! \param id Integer index.
        //! \return Constant mass of the element.
        double  operator[](int id) const;

        // Modification methods

        //! \brief Absorbs a specific amount of a single elemental species.
        //! \param element The `Element::Species` to absorb.
        //! \param amount Absolute mass to add.
        //! \throws std::logic_error If amount is negative.
        void Absorb(Element::Species element, double mass);

        //! \brief Absorbs the specified fraction of another Gas object.
        //! \param input The Gas object to absorb from (unchanged).
		//! \param fraction The fraction of the total object to absorb
        void Absorb(const Gas & input,double fraction=1.0);

		//! \brief Depletes a specified amount of the gas's total mass, keeping abundance levels constant.
		//! \param amount Either the fraction(0.0 to 1.0) of mass to deplete, or the absolute mass
		//! \param type Enum flag which controls how `amount' is interpreted (either a Fraction or Mass) 
		//! \throws std::logic_error If amount is out of range.
		void Deplete(double amount=1.0, Move type=Move::Fraction);

		//! Overload of Deplete(mass,Move:Mass)
		void DepleteMass(double mass){Deplete(mass,Move::Mass);};
		//! Overload of Deplete(mass,Move:Fraction)
		void DepleteFraction(double fraction){Deplete(fraction,Move::Fraction);};
			
        // Static Transfer methods
        //! \brief Transfers gas from source to destination.
        //! \param source Gas to transfer from.
        //! \param destination Gas to transfer to.
        //! \param amount Either the fraction to transfer (0.0 to 1.0), or the absolute mass
        //! \param amount Either the fraction to transfer (0.0 to 1.0), or the absolute mass
        //! \throws std::logic_error If fraction is out of range.
        static void Transfer(Gas & source, Gas & destination, double amount = 1,Move type=Move::Fraction);
        
		//! Overload of Transfer(source,destination,mass,Move::Mass)
		static void TransferMass(Gas & source, Gas & destination, double mass){Transfer(source,destination,mass,Move::Mass);};


		//! Overload of Transfer(source,destination,mass,Move::Fraction)
		static void TransferFraction(Gas & source, Gas & destination, double fraction){Transfer(source,destination,fraction,Move::Fraction);};


    private:
        //! \brief Private constructor (use factory functions).
        Gas();
       
		double validateMovement(double amount, Move type,const std::string & callingFunction);

        //! \brief Stores absolute mass of each elemental species.
        std::vector<double> internalMassOf;

        //! \brief Flags caches for recomputation after modifications.
        void RegisterChanges();

        //! \brief Computes and caches total mass.
        void ComputeMass() const;

        //! \brief Computes and caches elemental composition.
        void ComputeComposition() const;

        // Internal caches (mutable for const methods)
        mutable bool massNeedsRecomputing;
        mutable bool compNeedsRecomputing;
        mutable std::vector<double> internalComposition;
        mutable double internal_Mass;
};