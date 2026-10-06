# Joint Analysis

## Pick 3 of the 10 categories. For each, pick a language that gives it to you for free and say what that language pays for it. "Python has dictionaries" isn't an answer. What does Python's dictionary cost in memory or in speed compared to what you built, and where would you notice?

### Python's String

The benefit of using python’s string design is that it can store the length with the object, can accommodate zero bytes, and stores through powers of 2 bytes per character which scales if needed. Consequently, the tradeoff is that the variable-size header for strings is around 48-72 bytes(64-bit system) and they are immutable. The function dt_str_append doubles the size in reallocation while python creates a separate string that joins the original and the appended string,str.join (Programming FAQ, 2026). Since python uses reference counting, the issue of hindering performance is when an old string is being used elsewhere or is being aliased and there's a copy of that string where it's being appended. Also, python refuses to use variable-width encoding which means that despite having 1 byte for each character in a string, if a full unicode or an emoji is appended(takes 4 bytes for storage), the whole string must be scaled up to enforce same byte-width for each character. Prioritizing speed when looking up character values over a string over memory because you're quadrupling the byte allocation for each character. One can only notice this when constructing a large string at a time and it builds when doing operations on a large string a piece at a time.

### Java’s reference

Java manages its objects' references at runtime, so failures explicitly addressed by dt_ref guards reading after release, releasing twice, and forgetting to release are not memory errors for Java because by its design a reachable object is never freed. Because of garbage collection, there are no dangling references, double releases, or need to free objects manually, opposite to what dt_ref has for memory allocation and deallocation using a 16-byte handle(cell pointer + release flag) and a 16-byte cell for the dt_value (Woltmann, 2024). The GC usually operates on a spare heap to run efficiently, so it is noticeable in a program holding a lot of small objects and `List<int>` uses several times the memory of an int[]. The collector can pause the program without the code's control. It is short, but they are not a sure-guarantee, becoming an issue for latency-dependent programs(Robson, 2016). Leaks are less likely to happen in Java because the error does not come with the implementation; dangling reference and double free does not occur in Java’s GC. For every reachable object or an active thread, Java's GC considers them as alive and active. Pushing items into a HashMap for caching purposes without removal conditions, the objects will remain reachable indefinitely. And, over time, the unused references accumulate until the application crashes. In the driver’s exit sweep, this would be reported as DT_ERR_LEAK, while Java is silent until the memory runs out of space to work with and throws `OutOfMemoryError`.

---
## You wrote the tag check in dt_value_as_int by hand. Some languages don't let you. They make the tagged union a language construct, so the compiler writes the check for you, refuses to compile a read that skips it, and refuses to compile a set of cases that misses one. Rust's enum and match work this way, and so do ML's datatypes and Swift's enumerations with associated values. What does the C version let you do that a compiler enforcing the check wouldn't, and is any of it worth wanting?

C enables us to read the payload without even checking the tag. v.as.string compiles whatever v.tag gives. With Rust’s function match, the payload is exposed inside the variant, so unchecked reads can't function as designed (Klabnik & Nichols, 2025). There’s no guard in the code that stops us from writing as `v.tag = DT_STR` on an integer while the constructor in rust  requires both to match to create a variant. Also, since we know the exact size and position of the tag and payload, controlling the layout also enables us to pack the tag into a spare pointer. None of these really matter as they are usually unused, or just points in the wrong direction. A check across dt_value_as_int, dt_value_as_enum, and dt_value_as_str is the reason for this. Comparing C to Rust in this manner makes Rust can do the same with a knowledge check using unsafe if you know what you’re doing and if you want to optimize memory since that’s what C does; to control how you allocate objects in memory.

---

## Your dt_map keeps insertion order separately from the hash buckets, which is memory spent on something no lookup uses. Argue the other side: describe a design that drops it, say what breaks, and say whether you'd ship it.


---
## Compare access after release with an allocation that remains unreleased at the driver's final check. What damage can each cause in a long-running server? How does that answer change for a command-line tool that exits in a second?

---



References:
- Ondřej Měkota. (2022, April 16). Ondřej Měkota | Brief Look at CPython String. Mkta.Eu. https://mkta.eu/2022/04/16/brief-look-at-cpython-string

- Woltmann, S. (2024, November 25). Java Object Headers and Compressed Class Pointers. HappyCoders; HappyCoders GmbH. https://www.happycoders.eu/java/object-headers-compressed-class-pointers/

- Robson, M. (2016, December 6). Part 1: Introduction to the G1 Garbage Collector. Redhat.Com; Red Hat. https://www.redhat.com/en/blog/part-1-introduction-g1-garbage-collector

- S. Klabnik and C. Nichols, "The Match Control Flow Construct," The Rust Programming Language. https://doc.rust-lang.org/stable/book/ch06-02-match.html

- GeeksforGeeks. (2025, December 2). Process creation and deletions in operating systems. GeeksforGeeks. https://www.geeksforgeeks.org/operating-systems/process-creation-and-deletions-in-operating-systems/

- HashMap in std::collections - Rust. (n.d.). https://doc.rust-lang.org/std/collections/struct.HashMap.html

- Python. (n.d.). Data model. Python Documentation 3.14.8. https://docs.python.org/3/reference/datamodel.html#objects-values-and-types
