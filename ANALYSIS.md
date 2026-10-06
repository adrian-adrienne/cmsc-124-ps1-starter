# Joint Analysis

## Pick 3 of the 10 categories. For each, pick a language that gives it to you for free and say what that language pays for it. "Python has dictionaries" isn't an answer. What does Python's dictionary cost in memory or in speed compared to what you built, and where would you notice?

### Python's String

The benefit of using python’s string design is that it can store the length with the object, can accommodate zero bytes, and stores through powers of 2 bytes per character which scales if needed. Consequently, the tradeoff is that the variable-size header for strings is around 48-72 bytes(64-bit system) and they are immutable. The function dt_str_append doubles the size in reallocation while python creates a separate string that joins the original and the appended string,str.join (Programming FAQ, 2026). Since python uses reference counting, the issue of hindering performance is when an old string is being used elsewhere or is being aliased and there's a copy of that string where it's being appended. Also, python refuses to use variable-width encoding which means that despite having 1 byte for each character in a string, if a full unicode or an emoji is appended(takes 4 bytes for storage), the whole string must be scaled up to enforce same byte-width for each character. Prioritizing speed when looking up character values over a string over memory because you're quadrupling the byte allocation for each character. One can only notice this when constructing a large string at a time and it builds when doing operations on a large string a piece at a time.

### Java’s reference

Java manages its objects' references at runtime, so failures explicitly addressed by dt_ref guards reading after release, releasing twice, and forgetting to release are not memory errors for Java because by its design a reachable object is never freed. Because of garbage collection, there are no dangling references, double releases, or need to free objects manually, opposite to what dt_ref has for memory allocation and deallocation using a 16-byte handle(cell pointer + release flag) and a 16-byte cell for the dt_value (Woltmann, 2024). The GC usually operates on a spare heap to run efficiently, so it is noticeable in a program holding a lot of small objects and `List<int>` uses several times the memory of an int[]. The collector can pause the program without the code's control. It is short, but they are not a sure-guarantee, becoming an issue for latency-dependent programs(Robson, 2016). Leaks are less likely to happen in Java because the error does not come with the implementation; dangling reference and double free does not occur in Java’s GC. For every reachable object or an active thread, Java's GC considers them as alive and active. Pushing items into a HashMap for caching purposes without removal conditions, the objects will remain reachable indefinitely. And, over time, the unused references accumulate until the application crashes. In the driver’s exit sweep, this would be reported as DT_ERR_LEAK, while Java is silent until the memory runs out of space to work with and throws `OutOfMemoryError`.

### Python's Integers

Integers in Python are defined as a data object representation of numbers in an unlimited range, subject to the availability of (virtual) memory (Python, n.d.).  In what we built, a `long long` is 8 bytes of memory holding the number’s bits. When the code runs `a+b`, the CPU just adds the two values sitting in registers and that’s it. The compiler knows the type only from our declaration, which is why dt_value has tags. In Python, an integer is a data object that has an identity, type, and value. It is, therefore, self-describing. However, this costs more memory per number and extra work per operation having to lookup the type, and possibly allocating a new object making arithmetic slower. In exchange, Python integers grow as needed, so there are no overflows and no need for overflow checks. In problems or formulas where code can grow unpredictably, in what we built, C’s limitation with 8 bytes would refuse it, while Python would allow it to grow albeit higher memory use and runtime. The tradeoff here is that Python prioritizes unbounded range and freedom from overflow errors over speed and compact memory size.


## You wrote the tag check in dt_value_as_int by hand. Some languages don't let you. They make the tagged union a language construct, so the compiler writes the check for you, refuses to compile a read that skips it, and refuses to compile a set of cases that misses one. Rust's enum and match work this way, and so do ML's datatypes and Swift's enumerations with associated values. What does the C version let you do that a compiler enforcing the check wouldn't, and is any of it worth wanting?

C enables us to read the payload without even checking the tag. v.as.string compiles whatever v.tag gives. With Rust’s function match, the payload is exposed inside the variant, so unchecked reads can't function as designed (Klabnik & Nichols, 2025). There’s no guard in the code that stops us from writing as `v.tag = DT_STR` on an integer while the constructor in rust  requires both to match to create a variant. Also, since we know the exact size and position of the tag and payload, controlling the layout also enables us to pack the tag into a spare pointer. None of these really matter as they are usually unused, or just points in the wrong direction. A check across dt_value_as_int, dt_value_as_enum, and dt_value_as_str is the reason for this. Comparing C to Rust in this manner makes Rust can do the same with a knowledge check using unsafe if you know what you’re doing and if you want to optimize memory since that’s what C does; to control how you allocate objects in memory.

---

## Your dt_map keeps insertion order separately from the hash buckets, which is memory spent on something no lookup uses. Argue the other side: describe a design that drops it, say what breaks, and say whether you'd ship it.

Our dt_map stores each map_entry in two structures: a bucket chain for lookup and a doubly-linked insertion-order list. A design that drops it is Rust’s HashMap. According to Rust’s documentation, a hash map is implemented with quadratic probing and SIMD lookup. Its entries live in one slot array with no separate chains and no order list. By default, the hash map uses a randomly seeded hashing algorithm to provide resistance against HashDoS attacks.Without the order list, the output order is decided by the hash function and the table layout (HashMap in Std::Collections - Rust, n.d.). Since Rust randomly seeds the hash function, the same keys can have different orders in different runs. This breaks iteration order. You can no longer reliably traverse the map and print in the order of insertion since there is no list to track it. 

The question of shipping it depends on the purpose or use case. Rust specifically uses this design on purpose for security and speed. If the use case mostly consists of fast-lookups and disregards the insertion order, this design would be more efficient as it uses less memory and has faster speed since it doesn’t have to maintain two structures (bucket chain and insertion-order list). However, if the use case is predictable iteration, then I would not ship this design because insertion order is necessary. A randomized order would not be useful for cases in which a developer has to check if an output matches the input.

---
## Compare access after release with an allocation that remains unreleased at the driver's final check. What damage can each cause in a long-running server? How does that answer change for a command-line tool that exits in a second?


In C, a dangling reference retains an address after release and reading that address can cause undefined behavior. This can return old data or terminate. In our dt_ref module, this is prevented by checking the released flag and returning DT_ERR_RELEASED if true so that the driver can stop safely. An unreleased allocation on the other hand, still has an owner at the final check which means a reference still held its cell when the program ended. It is simply a leak. The driver reports DT_ERR_LEAK before cleanup. In a long-running server, access after release would break correctness or even crash the program while running due to undefined behavior. Since the server will continuously run and the leaked allocations remain reserved until the program ends, the leaks would just accumulate until the memory runs out. For a command-line tool that exits in a second, like this problem set for example, the answer for access after release remains the same. For unreleased allocation however, for this assignment, the driver sweeps at exit. This means it iterates through all dt_ref and checks if it is released. It reports DT_ERR_LEAK for all unreleased refs and calls dt_ref_destroy to clean up those cells. It is good to note though that this sweep is only for demonstrating leaks in the assignment. Most modern operating systems automatically reclaim all resources used after a process ends, so leaks do not persist across runs of short-lived tools (GeeksforGeeks, 2025).

---



References:
- Ondřej Měkota. (2022, April 16). Ondřej Měkota | Brief Look at CPython String. Mkta.Eu. https://mkta.eu/2022/04/16/brief-look-at-cpython-string

- Woltmann, S. (2024, November 25). Java Object Headers and Compressed Class Pointers. HappyCoders; HappyCoders GmbH. https://www.happycoders.eu/java/object-headers-compressed-class-pointers/

- Robson, M. (2016, December 6). Part 1: Introduction to the G1 Garbage Collector. Redhat.Com; Red Hat. https://www.redhat.com/en/blog/part-1-introduction-g1-garbage-collector

- S. Klabnik and C. Nichols, "The Match Control Flow Construct," The Rust Programming Language. https://doc.rust-lang.org/stable/book/ch06-02-match.html

- GeeksforGeeks. (2025, December 2). Process creation and deletions in operating systems. GeeksforGeeks. https://www.geeksforgeeks.org/operating-systems/process-creation-and-deletions-in-operating-systems/

- HashMap in std::collections - Rust. (n.d.). https://doc.rust-lang.org/std/collections/struct.HashMap.html

- Python. (n.d.). Data model. Python Documentation 3.14.8. https://docs.python.org/3/reference/datamodel.html#objects-values-and-types
