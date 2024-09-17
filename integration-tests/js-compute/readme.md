# Writing Integration Tests for JS-Compute-Runtime

- Create a folder within [fixtures](./fixtures) which will contain the test Fastly Compute application
- Create a javascript or typescript file within the folder with the same name as the folder, this file should contain the test Fastly Compute application.
- Create a fastly.toml.in file which contains any backends that the application requires. This file should adhere to the format defined at <https://developer.fastly.com/reference/fastly-toml/>
- Create a tests.json file which following the format defined below.

## Fields in tests.json

tests.json is a file containing a JSON Object which contains all the test requests and assertions for the Fastly Compute application being tested.

The keys in the root object are used as the name of the tests.

### environments

An array of strings which states in what environments the test should run.
Valid strings which can be in the array are:

- 'viceroy' - This states that the test should run in the Viceroy environment.
- 'compute' - This states that the test should run in the Fastly Compute environment.

### downstream_request

An Object which determines how the request to the test application should be configured.
The Object can contain the four properties:

- method
- pathname
- body
- headers

#### method

A string which is the name of the method that the request should use.
E.G. `"GET"` will make the request be a GET request.

#### pathname

A string which is the path and querystring that the request should use.
E.G. `"/"` will make the request go to `"/"` and `"/?colour=blue"` will make the request go to `"/?colour=blue"`.

#### body

A string which would be used as the request's body.

#### headers

An object which contains string keys and values that would each be used as a request header.
E.G. `{"a": "1", "b": "2" }` will set two headers on the request, a header named `a` with the value `1` and a header named `b` with the value `2`.

### local_upstream_requests

An Array of Objects which are used to assert against requests that the test application may have made during it's execution.
The Objects can contain the five properties:

- method
- pathname
- body
- headers
- timing

#### method

A string which is the name of the method to assert that the test application request has used during it's execution.
E.G. `"GET"` will assert that the upstream request was a GET request.

#### pathname

A string which is the path and querystring to assert that the test application request has used during it's execution.
E.G. `"/?colour=blue"` will assert that the upstream request was made to `"/?colour=blue"`.

#### body

A string which is the body to assert that the upstream request has used during it's execution.

#### headers

An object which contains string keys and values that would each be used as an assertion that the test application request has each header set during it's execution.
E.G. `{"a": "1", "b": "2" }` will assert that the upstream Request only has the headers named `a` with the value `1` and a header named `b` with the value `2`.

#### timing

A string set to `"afterDownstreamrequest"` to assert that the test application request happened after the downstream request.
If ommited, then no asserton will be made on when the upstream request happened.

### downstream_response

An Object which is used to assert against the response that the test application responded with.
The Object can contain the three properties:

- status
- body
- headers

#### status

A number which is the status to assert that the test application response has used.
E.G. `204` will assert that the response had a status set to 204.

#### body

A string or array which is the body to assert that the upstream request has used during it's execution.
If a string then the assertion is done on the entire response body.
If an array then assertions are done on each individual chunk received from response as a stream.

#### headers

An array which contains arrays, where each array contains two strings, the first is the header name and the second is the header value.
E.G. `{"a": "1", "b": "2" }` will assert that the response only has the headers named `a` with the value `1` and a header named `b` with the value `2`.

### logs

An array which contains strings used to assert against any logs emitted by the test application during it's execution.
E.G. `["ComputeLog :: Hello!"]` will assert that the application emitted a log-line containing only the string `"ComputeLog :: Hello!"`.

## Example tests.json file

```json
{
  "test for GET /example": {
    "environments": ["viceroy", "compute"],
    "downstream_request": {
      "method": "GET",
      "pathname": "/example",
      "headers": ["food", "carrot"]
    },
    "downstream_response": {
      "status": 200,
      "headers": [
        ["carrot", "yes"],
        ["potato", "no"]
      ],
      "body": "response from /example"
    },
    "local_upstream_requests": [
      {
        "method": "POST",
        "pathname": "/upstream-after-downstream",
        "headers": [["test-header", "test-header-value"]],
        "body": "pow wow",
        "timing": "afterDownstreamrequest"
      },
      {
        "method": "GET",
        "pathname": "/upstream"
      }
    ]
  }
}
```
