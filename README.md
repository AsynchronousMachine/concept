# AsynchronousMachine

Refer to *README.md* in git branch ***concept-dataobject***!

# TODOs

In branch ***concept-dataobject***:

* [ ] Test to replace `std::result_of_t<Visitor(D)> get(Visitor visitor) const` by `auto get(Visitor visitor) const`
* [ ] Change interface from Visitor to make a copy of _content to give a reference to _content
* [ ] Add special use cases for indentionally misuse of get/set with `const_cast<>()`

In all branches:

* [ ] Update to changes (1), (2) and (3) which have been made

---

Avoiding of unnecessary thread starts if less events are triggered then threads available:

* [ ] Try `void notify() { _cv.notify_one(); }` instead of `void notify() { _cv.notify_all(); }`!

---

Clang reflection interfaces ...


