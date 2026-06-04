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

async function sendPost() {
    const input = document.getElementById("postInput");
    const body = input.value;
    try {
        const response = await fetch("/echo",
            {
                method: "POST",
                headers: { "Content-Type": "text/plain" },
                body: body
            }
        );
        const text = await response.text();
        setResponse(text);
    } catch (error) {
        console.error(error);
        setResponse("request failed");
    }
}