#ifndef ORGANISM_H
#define ORGANISM_H
#include <concepts>

template <bool sp1_eats_m, bool sp1_eats_p, bool sp2_eats_m, bool sp2_eats_p>
concept two_plants_cant_meet = 
    sp1_eats_m   
    || sp1_eats_p 
    || sp2_eats_m 
    || sp2_eats_p;

template <typename species_t, bool can_eat_meat, bool can_eat_plants> 
requires std::equality_comparable<species_t>
class Organism {
    private:
        uint64_t vitality;
        species_t species;
    public:
    // Constructors
        constexpr Organism(species_t const &species, uint64_t vitality) 
         : species(species), vitality(vitality) {}

    // Accessors 
        constexpr uint64_t get_vitality() const {
            return vitality;
        }

        constexpr const species_t &get_species() const {
            return species;
        }

    // Methods defined by us for use in the encounter() method.
    // Returns true if an organism is dead.
        constexpr bool dead() {
            return !vitality;
        }

    // Returns true if the organism.
        constexpr bool plant() {
            return !(can_eat_meat || can_eat_plants);
        }

    // Returns true if this organism can eat organism given.
    // Determined by specialization, default is Omnivore.
        template <
            typename species_t_other, 
            bool can_eat_meat_other, 
            bool can_eat_plants_other
        > 
        constexpr bool can_I_eat(
            Organism<
            species_t_other, 
            can_eat_meat_other, 
            can_eat_plants_other
            > organism
        ) {
            return (
                can_eat_meat && !organism.plant()) 
                || can_eat_plants && organism.plant();
        }

    // Returns an object representing an organism after an encounter.
        constexpr Organism after(uint64_t new_vitality) {
            return Organism(species, new_vitality);
        }
    // By default it will return a copy of the organism.
        constexpr Organism after() {
            return Organism(species, vitality);
        }
};

// This might be actually fine?
template <typename species_t>
using Plant = Organism<species_t, false, false>;

template <typename species_t>
using Herbivore = Organism<species_t, false, true>;

template <typename species_t>
using Carnivore = Organism<species_t, true, false>;

template <typename species_t>
using Omnivore = Organism<species_t, true, true>;

namespace {
    // Function determining whether two organisms can have sex
    // in point 4.
    template < 
        typename species_t, 
        bool sp1_eats_m, 
        bool sp1_eats_p, 
        bool sp2_eats_m, 
        bool sp2_eats_p
    > // template
    constexpr bool // return type
    sex(
        Organism<species_t, sp1_eats_m, sp1_eats_p> organism1,
        Organism<species_t, sp2_eats_m, sp2_eats_p> organism2
    ) /*arguments*/ {
        return 
            organism1.species() == organism2.species() 
            && sp1_eats_m == sp2_eats_m
            && sp1_eats_p == sp2_eats_p
    } // function body

    // Function determining the outcome of an encounter in points 5, 6, 7, 8.
    template < 
        typename species_t, 
        bool sp1_eats_m, 
        bool sp1_eats_p, 
        bool sp2_eats_m, 
        bool sp2_eats_p
    > // template
    constexpr auto // return type
    eating_each_other(
        Organism<species_t, sp1_eats_m, sp1_eats_p> organism1,
        Organism<species_t, sp2_eats_m, sp2_eats_p> organism2
    ) /*arguments*/ {
        bool can1eat2 = organism1.can_I_eat(organism2);
        bool can2eat1 = organism1.can_I_eat(organism2);
        uint64_t v1 = organism1.get_vitality();
        uint64_t v2 = organism2.get_vitality();
        // Organism1 attempts to eat organism2.
        if (can1eat2) {
            if (organism2.plant()) 
                return (organism1.after(v1 + v2), organism2.after(0));
            if (v1 > v2)
                return (organism1.after(v1 + v2/2), organism2.after(0));
        }
        // It has failed.
        // Now organism2 attempts to eat organism1.
        if (can2eat1) {
            if (organism1.plant()) 
                return (organism1.after(0), organism2.after(v1 + v2));
            if (v2 > v1) 
                return (organism1.after(0), organism2.after(v1/2 + v2));
        }
        // Organisms can't eat each other, they either 
        // kill each other or do nothing 
        if (can1eat2 && can2eat1)
            return (organism1.after(0), organism2.after(0));

        return (organism1.after(), organism2.after());
    } // function body

}

template < 
    typename species_t, 
    bool sp1_eats_m, 
    bool sp1_eats_p, 
    bool sp2_eats_m, 
    bool sp2_eats_p
> // template
requires two_plants_cant_meet<
    sp1_eats_m, 
    sp1_eats_p, 
    sp2_eats_m, 
    sp2_eats_p
> // Two plants can't meet requirement.
constexpr std::tuple<
    Organism<species_t, sp1_eats_m, sp1_eats_p>,
    Organism<species_t, sp2_eats_m, sp2_eats_p>,
    std::optional<Organism<species_t, sp1_eats_m, sp1_eats_p>>
> // return type
encounter(
    Organism<species_t, sp1_eats_m, sp1_eats_p> organism1,
          Organism<species_t, sp2_eats_m, sp2_eats_p> organism2
) {
    if (organism1.dead() || organism2.dead()) 
        return (organism1.after(), organism2.after());

    if (sex(organism1, organism2)) 
        return (
            organism1.after(), 
            organism2.after(), 
            organism1.after(
                (organism1.get_vitality() + organism2.get_vitality)/2
            )
        );
    
    return eating_each_other(organism1, organism2);  
}

template <
    typename species_t, 
    bool sp1_eats_m, 
    bool sp1_eats_p, 
    typename ... Args
> // template
constexpr Organism<species_t, sp1_eats_m, sp1_eats_p> // return type
encounter_series(
    Organism<species_t, sp1_eats_m, sp1_eats_p> organism1, Args ... args
) /*arguments*/ {
    Organism<species_t, sp1_eats_m, sp1_eats_p> organism1_after = organism1.
    return (organism1_after = get<0>(encounter(organism1_after, args)));
} // function body

#endif