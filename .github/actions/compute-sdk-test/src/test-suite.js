
class TestConfig {

  constructor(testBase) {
    this.testBase = testBase;
  }

  get fixtureBase() {
    return `${this.testBase}/fixtures`;
  }

  get buildScript() {
    return `${this.testBase}/build-one.sh`;
  }

  get replaceHostScript() {
    return `${this.testBase}/replace-host.sh`;
  }

  wasmPath(testName) {
    return `${this.fixtureBase}/${testName}/${testName}.wasm`;
  }

  fastlyTomlPath(testName) {
    return `${this.fixtureBase}/${testName}/fastly.toml`;
  }

  fastlyTomlInPath(testName) {
    return `${this.fixtureBase}/${testName}/fastly.toml.in`;
  }

  testJsonPath(testName) {
    return `${this.fixtureBase}/${testName}/tests.json`;
  }
};

export default { TestConfig };
