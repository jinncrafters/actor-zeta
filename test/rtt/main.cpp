#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one cpp file
#include <catch2/catch.hpp>

#include <actor-zeta/detail/rtt.hpp>

#include "classes.hpp"

using actor_zeta::detail::rtt;

TEST_CASE("rt_tuple") {
    SECTION("DC can be created from an arbitrary set of objects ") {
        rtt t(std::string("123"), 42, true, X{17, std::vector<int>{1, 2, 3}});
        CHECK(t.size() == 4);
        CHECK(t.get<std::string>(0) == std::string("123"));
        CHECK(t.get<int>(1) == 42);
        CHECK(t.get<bool>(2) == true);
        CHECK(t.get<X>(3).a == 17);
        CHECK(t.get<X>(3).b == std::vector<int>{1, 2, 3});
    }

    SECTION("DK created from an empty set of elements is empty") {
        const auto t = rtt{};
        CHECK(t.empty());
    }

    SECTION("The size of a DC created from an empty set of elements is zero ") {
        const auto t = rtt{};
        CHECK(t.size() == 0);
    }

    SECTION("The volume of a DC created from an empty set of elements is zero ") {
        const auto t = rtt{};
        CHECK(t.volume() == 0);
    }

    SECTION("The capacity of a DC created from an empty set of elements is zero ") {
        const auto t = rtt{};
        CHECK(t.capacity() == 0);
    }

    SECTION("DK created from a non-empty set of elements is not empty ") {
        const auto t = rtt(42, 3.14);
        CHECK(not t.empty());
    }

    SECTION("The size of a DC created from a non-empty set of elements is equal to the number of these elements ") {
        const auto t = rtt(std::string("123"), 42, 2.71, false);
        CHECK(t.size() == 4);
    }

    SECTION("The volume of a DC created from a set of elements is equal to the volume of a DC created by sequential insertion of the same elements into the end of an empty DC. ") {
        const auto constructed = rtt(42, 3.14, std::string("123"), true);

        auto pushed_back = rtt{42,3.14,std::string("123",true);

        CHECK(constructed.volume() == pushed_back.volume());
    }

    SECTION("The capacity of a recreation center created from a set of elements is greater than or equal to the volume of this recreation center ") {
        const auto t = rtt(std::vector<std::size_t>{}, 42, X{}, 3.14);
        CHECK(t.capacity() >= t.volume());
    }

    SECTION("The size of the DC is equal to the number of elements it contains") {
        const auto some_vector = std::vector<std::size_t>{};
        const auto some_integer = 42;
        const auto some_struct = X{};
        const auto some_floating = 3.14;

        rtt t(some_vector, some_integer, some_struct, some_floating);
        CHECK(t.size() == 4);
    }

    SECTION("Arbitrary objects can be inserted at the end of the DC") {
        rtt t(X{17, std::vector<int>{1, 2, 3}},true,std::string("123"));

        CHECK(t.size() == 3);
        CHECK(t.get<X>(0).a == 17);
        CHECK(t.get<X>(0).b == std::vector<int>{1, 2, 3});
        CHECK(t.get<bool>(1) == true);
        CHECK(t.get<std::string>(2) == std::string("123"));
    }

    SECTION("The exception thrown by the inserted object does not result in leaks ") {
        REQUIRE(kamikaze::instances_count == 0);
        REQUIRE_THROWS_AS(rtt(kamikaze{}, kamikaze{}, kamikaze{}), std::runtime_error);
        CHECK(kamikaze::instances_count == 0);
    }

    SECTION("Objects stored in the DC can be changed externally ") {
        rtt t(true, std::vector<char>{'a', 'b', 'c'}, 10);

        auto& vector = t.get<std::vector<char>>(1);

        vector.push_back('q');
        CHECK(t.get<std::vector<char>>(1) == std::vector<char>{'a', 'b', 'c', 'q'});

        vector.clear();
        CHECK(t.get<std::vector<char>>(1).empty());
    }

    SECTION("The DC transfer constructor transfers elements of one DC into another, rather than inserting one DC inside another DC") {
        rtt moved(2.71, 3, false);
        rtt constructed(std::move(moved));
        CHECK(constructed.size() == 3);
    }

    SECTION("The DK copy constructor copies the elements of one DK to another, rather than inserting one DK inside another DK") {
        rtt copied(2.71, 3, false);
        rtt constructed(copied);
        CHECK(constructed.size() == 3);
    }

    SECTION("DC may contain DC") {
        rtt t(rtt{1, 2}, rtt{3, 4});
        CHECK(t.size() == 2);
    }

    SECTION("Objects stored in the DC are transferred when transferring the DC") {
        REQUIRE(dummy::instances_count == 0);
        rtt one(dummy{});
        REQUIRE(dummy::instances_count == 1);

        rtt another(std::move(one));
        CHECK(dummy::instances_count == 1);

        one = std::move(another);
        CHECK(dummy::instances_count == 1);
    }

    struct throw_on_move {
        throw_on_move() = default;
        throw_on_move(const throw_on_move&) = default;
        throw_on_move& operator=(const throw_on_move&) = default;
        throw_on_move(throw_on_move&&) {
            throw std::runtime_error(u8"Пока!");
        }
        throw_on_move& operator=(throw_on_move&&) = default;
        ~throw_on_move() = default;
    };

    SECTION("Exception when moving internal objects does not result in leaks") {
        const throw_on_move thrower{};

        REQUIRE(dummy::instances_count == 0);
        {
            rtt t;
            t.push_back(dummy{});
            t.push_back(thrower);
            REQUIRE(dummy::instances_count == 1);

            REQUIRE_THROWS_AS(t.reserve(t.capacity() + 1), std::runtime_error);
            CHECK(dummy::instances_count == 1);
        }
        CHECK(dummy::instances_count == 0);
    }

    SECTION("The DK copy constructor creates new instances of objects stored in the copied DK and destroys them when the created DK copy is destroyed") {
        REQUIRE(dummy::instances_count == 0);
        {
            rtt initial(dummy{});

            REQUIRE(dummy::instances_count == 1);
            {
                rtt copy(initial);
                CHECK(dummy::instances_count == 2);
            }
            CHECK(dummy::instances_count == 1);
        }
        CHECK(dummy::instances_count == 0);
    }

    SECTION("The operator of the copying assignment of the DC creates new instances of objects stored in the copied DC and destroys them when this copy of the DC is destroyed. ") {
        REQUIRE(dummy::instances_count == 0);
        {
            rtt initial(dummy{});

            REQUIRE(dummy::instances_count == 1);
            {
                rtt copy(dummy{});
                copy = initial;
                CHECK(dummy::instances_count == 2);
            }
            CHECK(dummy::instances_count == 1);
        }
        CHECK(dummy::instances_count == 0);
    }

    SECTION("Objects stored in the DC are destroyed by copy assignment") {
        REQUIRE(dummy::instances_count == 0);
        rtt initial(dummy{});
        REQUIRE(dummy::instances_count == 1);

        rtt other(1, 3.14, true);
        initial = other;
        CHECK(dummy::instances_count == 0);
    }

    SECTION("Objects stored in the DC are destroyed during transfer assignment") {
        REQUIRE(dummy::instances_count == 0);
        rtt initial(dummy{});
        REQUIRE(dummy::instances_count == 1);

        rtt other(1, 3.14, true);
        initial = std::move(other);
        CHECK(dummy::instances_count == 0);
    }

    SECTION("The transfer constructor leaves the portable DC instance empty") {
        rtt source(13, 3.14, true);
        rtt destination(std::move(source));
        CHECK(source.empty());
    }

    SECTION("The takeover assignment operator leaves the portable copy of the DC empty") {
        rtt source(std::string("move me"));
        rtt destination(std::string("some items here"));
        destination = std::move(source);
        CHECK(source.empty());
    }

    SECTION("The size of the DC source after transfer is equal to zero") {
        rtt source(13, 3.14, true);
        rtt destination(std::move(source));
        CHECK(source.size() == 0);
    }

    SECTION("The size of the DC-source after carrying assignment is equal to zero ") {
        rtt source(std::string("move me"));
        rtt destination(std::string("some items here"));
        destination = std::move(source);
        CHECK(source.size() == 0);
    }

    SECTION("The volume of the DC source after transfer is equal to zero") {
        rtt source(13, 3.14, true);
        rtt destination(std::move(source));
        CHECK(source.volume() == 0);
    }

    SECTION("The volume of the DC-source after carrying assignment is equal to zero") {
        rtt source(std::string("move me"));
        rtt destination(std::string("some items here"));
        destination = std::move(source);
        CHECK(source.volume() == 0);
    }

    SECTION("Copying a DC leads to the creation of real copies of the objects stored in it") {
        rtt initial(std::string("cat"));

        auto copy = rtt(initial);
        REQUIRE(copy.get<std::string>(0) == "cat");

        initial.get<std::string>(0).append("harsis");
        CHECK(copy.get<std::string>(0) == "cat");
    }

    SECTION("The copying assignment of the DC leads to the creation of real copies of the objects stored in it.") {
        rtt initial(std::vector<std::string>(1, "dog"));

        rtt copy(std::string("qwerty"));
        copy = initial;
        REQUIRE(copy.get<std::vector<std::string>>(0) == std::vector<std::string>(1, "dog"));

        initial.get<std::vector<std::string>>(0).push_back("horse");
        CHECK(copy.get<std::vector<std::string>>(0) == std::vector<std::string>(1, "dog"));
    }

    SECTION("As a result of copy assignment, the volume of the copy is equal to the volume of the original") {
        rtt initial(std::string("cat"), 5, 3.14, std::vector<int>{1, 2});

        rtt copy(true, 2.71);
        copy = initial;
        CHECK(initial.volume() == copy.volume());
    }

    SECTION("As a result of copy assignment, the size of the copy is equal to the size of the original") {
        rtt initial(std::string("cat"), 5, 3.14, std::vector<int>{1, 2});

        rtt copy(17);
        copy = initial;
        CHECK(initial.size() == copy.size());
    }

    SECTION("The result of a copy assignment of an empty DC is an empty DC") {
        rtt initial;

        rtt copy(std::string("123"), 'a');
        copy = initial;
        CHECK(copy.empty());
    }

    SECTION("Throws an exception when trying to copy a non-copyable object") {
        rtt initial(std::make_unique<int>(5));
        CHECK_THROWS_AS(rtt copy(initial), std::runtime_error);
    }

    SECTION("Exception when copying objects stored in the DC does not lead to leaks") {
        REQUIRE(dummy::instances_count == 0);
        {
            rtt t(dummy{}, std::make_unique<int>(17), dummy{});
            REQUIRE(dummy::instances_count == 2);

            REQUIRE_THROWS_AS(rtt copy(t), std::runtime_error);
            CHECK(dummy::instances_count == 2);
        }
        CHECK(dummy::instances_count == 0);
    }

    SECTION("Exception during copy assignment of objects stored in the DC does not lead to leaks") {
        REQUIRE(dummy::instances_count == 0);
        {
            rtt t(dummy{}, std::make_unique<int>(17));
            rtt copy(dummy{});
            REQUIRE(dummy::instances_count == 2);

            REQUIRE_THROWS_AS(copy = t, std::runtime_error);
        }
        CHECK(dummy::instances_count == 0);
    }

    SECTION("Transferring DC assignment does not lead to leaks") {
        REQUIRE(dummy::instances_count == 0);
        {
            rtt t(dummy{});
            REQUIRE(dummy::instances_count == 1);

            rtt copy(dummy{});
            REQUIRE(dummy::instances_count == 2);

            copy = std::move(t);
        }
        CHECK(dummy::instances_count == 0);
    }

    SECTION("Knows how to give references to immutable values by their indentation in the DC") {
        const rtt t(std::string("123"), 42, true);
        auto int_offset = t.offset(1);
        CHECK(t.get_by_offset<int>(int_offset) == 42);
    }

    SECTION("Knows how to give links to mutable values by their indentation in DC") {
        auto old_value = 42;
        auto new_value = 99;

        rtt t(std::string("123"), old_value, true);
        auto int_offset = t.offset(1);

        REQUIRE(t.get<int>(1) == old_value);
        t.get_by_offset<int>(int_offset) = new_value;

        CHECK(t.get<int>(1) == new_value);
    }

    SECTION("swap") {
        auto t = rtt(1, 3.14);
        auto u = rtt(std::string("qwe"), true);

        swap(t, u);
    }
}
