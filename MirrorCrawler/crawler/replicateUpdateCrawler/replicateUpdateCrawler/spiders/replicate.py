import scrapy
from urllib.parse import quote_plus
import logging
import json


class ReplicateSpider(scrapy.Spider):
    name = 'replicate'
    allowed_domains = ['replicate.npmjs.com']
    base_url = 'https://replicate.npmjs.com/registry/'
    # base_url = 'https://registry.npmmirror.com/'
    changes_url = 'https://replicate.npmjs.com/registry/_changes'


    def start_requests(self):

        with open(self.settings.get('LAST_SEQ_FILE'), 'r') as f:
            last_seq = json.load(f)['last_seq']
        logging.critical(f'LOAD LAST SEQ: {last_seq}')

        yield scrapy.Request(url='%s?last-event-id=%d&limit=1000' % (self.changes_url, last_seq),
                            callback=self.parse_changes,
                            meta={'download_timeout': 86400})

    def parse_changes(self, response):
        response_json = response.json()
        results = response_json['results']
        last_seq = response_json['last_seq']
        with open(self.settings.get('LAST_SEQ_FILE'), 'w') as f:
            json.dump({'last_seq': last_seq}, f)
        # logging.info(f'NEW LAST SEQ: {last_seq}')

        for i in results:
            if 'deleted' in i and i['deleted']:
                pass
                # yield i
            else:
                doc_id = i['id']
                changes_rev = i['changes'][0]['rev']
                yield scrapy.Request(url=self.base_url + quote_plus(doc_id),
                                    callback=self.parse,
                                    cb_kwargs={'changes_rev': changes_rev})
                
        if len(results) != 0:
            logging.critical(f'LOAD LAST SEQ: {last_seq}')
            yield scrapy.Request(url='%s?last-event-id=%d&limit=1000' % (self.changes_url, last_seq),
                            callback=self.parse_changes,
                            meta={'download_timeout': 86400})

    def parse(self, response, changes_rev):
        response_json = response.json()
        return_json = dict()
        return_json['changes_rev'] = changes_rev
        return_json['response_json'] = response_json
        yield return_json
