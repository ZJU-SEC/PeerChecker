FROM m.daocloud.io/docker.io/python:3
WORKDIR /app
COPY requirements.txt .
RUN pip install -r requirements.txt -i https://pypi.tuna.tsinghua.edu.cn/simple
COPY . .
WORKDIR /app/replicateUpdateCrawler
ENTRYPOINT ["scrapy", "crawl", "replicate"]
