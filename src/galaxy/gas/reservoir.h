#pragma once
#include "gas.h"

/**
 * @brief Defines a fractional temperature scale for gas, where 0 represents 'cold' and 1 represents 'hot'.
 */
enum class Temperature
{
	Cold=0, //temperature `scale' is a fractional mapping between cold and hot. Temperature::Cold is essentially an alias for temperature = 0..
	Hot=1
};

/**
 * @brief Represents a gas reservoir with two temperature components: Cold and Hot.
 *
 * This class models a mixed gas system where gas can be conceptually separated
 * into a 'Cold' component and a 'Hot' component
 */
class GasReservoir
{
	public:
		/**
         * @brief The 'hot', non-star-forming component of the gas in the reservoir
         */
		Gas Hot;
		/**
         * @brief The 'cold' star-forming component of the gas in the reservoir.
         */
		Gas Cold;

		// --- Interface ---

			/**
			 * @brief Calculates the total mass of gas within the reservoir.
			 * @return The sum of the masses of the Cold and Hot gas components.
			 */
			double TotalMass() const;

			/**
			 * @brief Calculates the current effective temperature of the reservoir.
			 * @return The temperature as a double between 0.0 (all cold) and 1.0 (all hot).
			 * Returns 0.0 if the reservoir has no mass.
			 */
			double GetTemperature() const;
			/**
			 * @brief Absorbs gas from another GasReservoir, adding its Cold and Hot components
			 * to this reservoir's respective components. 
			 * @details Alters the temperature of the GasReservoir unless the source has the same temperature
			 * @param sourceGas The (unmodified) GasReservoir to absorb gas from.
			 */
			void Absorb(const GasReservoir & sourceGas);
			

			/**
			 * @brief Absorbs a single Gas object into this reservoir, distributing its mass
			 * between the Cold and Hot components as if it were a reservoir with the specified temperature
			 * @param sourceGas The Gas object to absorb.
			 * @param sourceTemp The temperature (0.0 to 1.0) at which the source gas is absorbed.
			 * A value of 0.0 means all mass goes to Cold; 1.0 means all to Hot.
			 * Values in between are distributed fractionally.
			 */
			void Absorb(const Gas & sourceGas, double sourceTemp);

			/**
			 * @brief Removes gas from the reservoir without altering the temperature
			 *
			 * The `amount` specifies either a mass or a fraction to remove,
			 * and the `type` determines how it's calculated.
			 * If `Move::Mass`, the amount is removed proportionally based on the reservoir's
			 * current temperature. If `Move::Fraction`, the amount is a fraction
			 * of the Cold and Hot components to remove.
			 *
			 * @param amount The quantity to deplete (mass or fraction). Defaults to 1.
			 * @param type The type of depletion (Move::Mass or Move::Fraction). Defaults to Move::Fraction.
			 */
			void Deplete(double amount=1, Move type = Move::Fraction);

			/**
			 * @brief Depletes gas specifically from either the Cold or Hot component of the reservoir.
			 * @param temperature Specifies whether to deplete from the Temperature::Cold or Temperature::Hot component.
			 * @param amount The quantity to deplete (mass or fraction). Defaults to 1.
			 * @param type The type of depletion (Move::Mass or Move::Fraction). Defaults to Move::Fraction.
			 */
			void Deplete(Temperature temperature,double amount=1, Move type = Move::Fraction); //an alias for Hot.Deplete / Cold.Deplete
			
		
			
			/**
			 * @brief Transfers gas from a source Gas object to a destination GasReservoir.
			 *
			 * The source gas is split between the destination's Cold and Hot components
			 * based on `sourceTemperature`. The `amount` specifies either a mass or a fraction
			 * to transfer from the source gas, which is then absorbed by the destination.
			 *
			 * @param source The Gas object from which to transfer gas.
			 * @param destination The GasReservoir to which the gas will be transferred.
			 * @param sourceTemperature The temperature (0.0 to 1.0) of the gas being transferred.
			 * @param amount The quantity to transfer (mass or fraction). Defaults to 1.
			 * @param type The type of transfer (Move::Mass or Move::Fraction). Defaults to Move::Fraction.
			 */
			static void Transfer(Gas & source, GasReservoir & destination, double sourceTemperature, double amount = 1,Move type=Move::Fraction);

			/**
			 * @brief Transfers gas from one GasReservoir to another GasReservoir.
			 *
			 * Gas is depleted from the `source` reservoir and absorbed by the `destination`
			 * reservoir, maintaining their respective temperature compositions.
			 * The `amount` specifies either a mass or a fraction to transfer.
			 *
			 * @param source The GasReservoir from which to transfer gas (temperature unchanged)
			 * @param destination The GasReservoir to which the gas will be transferred (temperature changes)
			 * @param amount The quantity to transfer (mass or fraction). Defaults to 1.
			 * @param type The type of transfer (Move::Mass or Move::Fraction). Defaults to Move::Fraction.
			 */
			static void Transfer(GasReservoir & source, GasReservoir & destination, double amount = 1,Move type=Move::Fraction);


		// --- Constructors and Factory functions
		
			/**
			 * @brief Constructs an empty GasReservoir.
			 * @details Equivalent to calling GasReservoir::Empty().
			 */
			GasReservoir(); 
			
			/**
			 * @brief Constructs a GasReservoir from a single Gas object, distributing its mass
			 * between the Cold and Hot components based on the specified temperature.
			 * @details Equivalent to calling GasReservoir::FromGas(source,temp), but does not perform temperature validation
			 * @param source The Gas object from which to initialize the reservoir.
			 * @param temp The (unvalidated) temperature (0.0 to 1.0) of the source gas, determining its distribution.
			 */
			GasReservoir(const Gas & source, double temp);

			/**
			 * @brief Constructs a GasReservoir directly from pre-defined Cold and Hot Gas components.
			 * @param cold The Gas object representing the cold component.
			 * @param hot The Gas object representing the hot component.
			 * @details Equivalent to calling GasReservoir::FromGas(cold, hot).
			 */
			GasReservoir(const Gas & cold, const Gas & hot);

			/**
			 * @brief Creates and returns an empty GasReservoir.
			 * @return A new GasReservoir instance with no mass in either the Cold or Hot component.
			 */
			static GasReservoir Empty();

			/**
			 * @brief Creates and returns a primordial GasReservoir with a specified total mass
			 * and temperature. The mass is distributed between Cold and Hot components
			 * based on the temperature, and both components will have primordial composition.
			 * @param mass The total mass of the primordial gas.
			 * @param temperature The temperature (0.0 to 1.0) of the primordial gas. Defaults to Settings.Thermal.PrimordialTemperature.
			 * @return A new GasReservoir instance initialized with primordial gas.
			 */
			static GasReservoir Primordial(double mass,double temperature = Settings.Thermal.PrimordialTemperature);

			/**
			 * @brief Creates and returns a GasReservoir from a single Gas object, distributing its mass
			 * between the Cold and Hot components based on the specified temperature.
			 * @details Differentiated from the equivalent constructor by validating the temperature 
			 * @param sourceGas The Gas object to use as the source for the reservoir.
			 * @param temperature The temperature (0.0 to 1.0) to assign to the source gas. Defaults to 0.
			 * @return A new GasReservoir instance initialized from the source gas.
			 */
			static GasReservoir FromGas(const Gas & sourceGas, double temperature = 0);

			/**
			 * @brief Creates and returns a GasReservoir directly from pre-defined Cold and Hot Gas components.
			 * @param cold The Gas object for the cold component.
			 * @param hot The Gas object for the hot component.
			 * @return A new GasReservoir instance with the specified cold and hot components.
			 */
			static GasReservoir FromGas(const Gas & cold, const Gas & hot);
		
		
		// --- Convenience Overloads (Temperature enum) ---
			// These functions delegate to their double-temperature counterparts,
			// providing a type-safe way to use the Temperature enum.

			/**
			 * @brief Constructs a GasReservoir from a single Gas object using a Temperature enum.
			 * @param source The Gas object from which to initialize the reservoir.
			 * @param temp The Temperature enum value (Cold or Hot) determining the distribution.
			 */
			GasReservoir(const Gas & source, Temperature temp) : GasReservoir(source,static_cast<double>(temp)){};

			/**
			 * @brief Absorbs a single Gas object using a Temperature enum to determine its distribution.
			 * @param sourceGas The Gas object to absorb.
			 * @param sourceTemp The Temperature enum value (Cold or Hot) determining absorption.
			 */
			void Absorb(const Gas & sourceGas, Temperature sourceTemp){Absorb(sourceGas,static_cast<double>(sourceTemp));};

			/**
			 * @brief Creates a primordial GasReservoir using a Temperature enum.
			 * @param mass The total mass of the primordial gas.
			 * @param temperature The Temperature enum value (Cold or Hot) for the primordial gas.
			 * @return A new GasReservoir instance initialized with primordial gas.
			 */
			static GasReservoir Primordial(double mass, Temperature temperature){return Primordial(mass,static_cast<double>(temperature));};

			/**
			 * @brief Creates a GasReservoir from a source Gas object using a Temperature enum.
			 * @param sourceGas The Gas object to use as the source.
			 * @param temperature The Temperature enum value (Cold or Hot) for the source gas.
			 * @return A new GasReservoir instance.
			 */
			static GasReservoir FromGas(const Gas & sourceGas, Temperature temperature){return FromGas(sourceGas,static_cast<double>(temperature));};

			/**
			 * @brief Transfers gas from a source Gas object to a destination GasReservoir using a Temperature enum.
			 * @param source The Gas object from which to transfer gas.
			 * @param destination The GasReservoir to which the gas will be transferred.
			 * @param temperature The Temperature enum value (Cold or Hot) of the gas being transferred.
			 * @param amount The quantity to transfer (mass or fraction). Defaults to 1.
			 * @param type The type of transfer (Move::Mass or Move::Fraction). Defaults to Move::Fraction.
			 */
			static void Transfer(Gas & source, GasReservoir & destination, Temperature temperature, double amount = 1,Move type=Move::Fraction){Transfer(source,destination,static_cast<double>(temperature),amount,type);};
	private:

		/**
         * @brief Calculates the distribution of an 'amount' into cold and hot components
         * based on a given temperature and move type.
         * @param amount The total quantity to balance (mass or fraction).
         * @param type The type of movement (Move::Mass or Move::Fraction).
         * @param temperature The temperature (0.0 to 1.0) used for balancing.
         * @return A std::pair where 'first' is the cold component's amount and 'second' is the hot component's amount.
         */
		static std::pair<double,double> TemperatureBalancer(double amount, Move type,double temperature);

		/**
         * @brief Validates that a given temperature value is within the physical range [0.0, 1.0].
         * @param temperature The temperature value to validate.
         * @throws std::logic_error If the temperature is outside the [0.0, 1.0] range.
         */
		static void inline TemperatureValidate(double temperature);
};