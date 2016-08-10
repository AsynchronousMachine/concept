# AsynchronousMachine

Refer to *README.md* in git branch ***concept-dataobject***!

# TODOs

In branch ***concept-dataobject***:

1. Test to replace std::result_of_t<Visitor(D)> get(Visitor visitor) const by auto get(Visitor visitor) const
2. Change interface from Visitor to make a copy of _content to give a reference to _content
3. Add special use cases for indentionally misuse of get/set with const_cast<>()

In all branches: Update all branches to the changes (1), (2) and (3) have been made

---

Clang reflection interfaces ...

---

Avoiding of unnecessary thread starts if less events are triggered then threads available 
