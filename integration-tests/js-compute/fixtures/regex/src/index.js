var o = {
  depth: 0,
  get f() {
      console.log(this.depth++);
      return this.f;
  }
};
o.f;

addEventListener("fetch", (req) => {});