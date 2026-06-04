async function callEndpoint(path) {
    try {
        const response = await fetch(path);
        const text = await response.text();
        setResponse(text);
    } catch (error) {
        setResponse("request failed");
    }
}

async function greet() {
    const input = document.getElementById("nameInput");
    const name = encodeURIComponent(input.value);
    const path = "/greet?name=" + name;

    console.log(path);

    try {
        const response = await fetch(path);
        const text = await response.text();
        setResponse(text);
    } catch (error) {
        console.error(error);
        setResponse("request failed");
    }
}

function setResponse(text) {
    document.getElementById("responseBox").textContent = text;
}