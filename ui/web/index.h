#pragma once

#include <string>

namespace NUI {

static const std::string INDEX = R"(
<!DOCTYPE html>
    <head>
        <title>opennap web player</title>
    </head>
    <body>

        <style>
            .selected {
                background-color:gray;
            }
            .table {
                width: 70%;
                border: 1px solid;
                margin: 8px 2px;
            }
            .button {
                width: 250px;
                height: 120px;
                background-color: blue;
                border: none;
                color: white;
                padding: 15px 15px;
                text-align: center;
                text-decoration: none;
                display: inline-block;
                font-size: 16px;
                margin: 8px 2px;
                cursor: pointer;
            }
            .textarea {
                width: 70%;
                border: 1px solid;
                resize: none;
                outline: none;
                margin: 8px 2px;
            }
        </style>
        
        <div>
          <div id="filelist"></div>
          <textarea id="status" class="textarea" rows="4" cols="50"></textarea>
          <div>
              <button onclick="onClick('up') " class="button">up</button>
              <button onclick="onClick('down') " class="button">down</button>
              <br>
              <button onclick="onClick('enter') " class="button">enter</button>
              <button onclick="onClick('exit') " class="button">exit</button>
              <br>
              <button onclick="onClick('play') " class="button">play</button>
              <button onclick="onClick('stop') " class="button">stop</button>
          </div>
        </div>

        <script>
            let host = location.host

            let onClick = async (path) => {
                try {
                    const response = await fetch(`http://${host}/${path}`,
                        { timeout: 100, method: "POST" }
                    );
                    if(response.status !== 200){
                        console.log(error);
                    }
                } catch (error) {
                    console.log(error);
                }
            }

            let fetchFileList = async () => {
                try {
                    const response = await fetch(`${host}/list`,
                        { timeout: 100 }
                    );
                    if (response.status === 200) {
                        const data = await response.json();

                        const table = document.createElement("table");
                        table.classList.add("table");
                        table.border = "1";

                        data.forEach(item => {
                            const row = document.createElement("tr");
                            const cell = document.createElement("td");
                            cell.textContent = item["name"];

                            if (item["current"]) {
                                cell.classList.add("selected");
                            }

                            row.appendChild(cell);
                            table.appendChild(row);

                            let fileList = document.getElementById("filelist");
                            fileList.innerHTML = ''
                            fileList.appendChild(table);
                        });
                    }
                } catch (error) {
                    console.log(error);
                }
                await setTimeout(fetchFileList, 1000)
            }

            fetchFileList();

            let fetchStaus = async () => {
                try {
                    const response = await fetch(`${host}/status`,
                        { timeout: 100 }
                    );
                    if (response.status === 200) {
                        const data = await response.text();
                        document.getElementById("status").value = data;
                    }
                } catch (error) {
                    console.log(error);
                }
                await setTimeout(fetchStaus, 1000)
            }

            fetchStaus();
        </script>

    </body>
</html>
)";

}
