namespace stage2 {

class structural_iterator {
public:
  const uint8_t* const buf;
  uint32_t *next_structural;
  uint8_t c{0};  // used to track the (structural) character we are looking at
  dom_parser_implementation &parser;

  really_inline structural_iterator(dom_parser_implementation &_parser, size_t next_structural_index)
    : buf{_parser.buf},
      next_structural{&_parser.structural_indexes[next_structural_index]},
      parser{_parser} {
  }
  really_inline char advance_char() {
    c = buf[*next_structural];
    next_structural++;
    return c;
  }
  really_inline char current_char() {
    return c;
  }
  really_inline char peek_char() {
    return buf[*next_structural];
  }
  really_inline const uint8_t* current() {
    return &buf[current_structural_index()];
  }
  really_inline size_t remaining_len() {
    return parser.len - current_structural_index();
  }
  really_inline uint32_t current_structural_index() {
    return *(next_structural-1);
  }
  template<typename F>
  really_inline bool with_space_terminated_copy(const F& f) {
    /**
    * We need to make a copy to make sure that the string is space terminated.
    * This is not about padding the input, which should already padded up
    * to len + SIMDJSON_PADDING. However, we have no control at this stage
    * on how the padding was done. What if the input string was padded with nulls?
    * It is quite common for an input string to have an extra null character (C string).
    * We do not want to allow 9\0 (where \0 is the null character) inside a JSON
    * document, but the string "9\0" by itself is fine. So we make a copy and
    * pad the input with spaces when we know that there is just one input element.
    * This copy is relatively expensive, but it will almost never be called in
    * practice unless you are in the strange scenario where you have many JSON
    * documents made of single atoms.
    */
    char *copy = static_cast<char *>(malloc(parser.len + SIMDJSON_PADDING));
    if (copy == nullptr) {
      return true;
    }
    memcpy(copy, buf, parser.len);
    memset(copy + parser.len, ' ', SIMDJSON_PADDING);
    bool result = f(reinterpret_cast<const uint8_t*>(copy), current_structural_index());
    free(copy);
    return result;
  }
  really_inline bool past_end(uint32_t n_structural_indexes) {
    return next_structural > &parser.structural_indexes[n_structural_indexes];
  }
  really_inline bool at_end(uint32_t n_structural_indexes) {
    return next_structural == &parser.structural_indexes[n_structural_indexes];
  }
  really_inline bool at_beginning() {
    return next_structural == &parser.structural_indexes[0];
  }
};

} // namespace stage2
