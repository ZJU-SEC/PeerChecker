# Define your item pipelines here
#
# Don't forget to add your pipeline to the ITEM_PIPELINES setting
# See: https://docs.scrapy.org/en/latest/topics/item-pipeline.html


# useful for handling different item types with a single interface
from itemadapter import ItemAdapter
from ibmcloudant.cloudant_v1 import CloudantV1
from ibm_cloud_sdk_core.api_exception import ApiException
import logging
import os


class ReplicateupdatecrawlerPipeline:
    def process_item(self, item, spider):
        return item


class CouchDBPipeline:

    def __init__(self, couchdb_admin, couchdb_password, couchdb_url, couchdb_db):
        os.environ['CLOUDANT_AUTH_TYPE'] = 'COUCHDB_SESSION'
        os.environ['CLOUDANT_USERNAME'] = couchdb_admin
        os.environ['CLOUDANT_PASSWORD'] = couchdb_password
        os.environ['CLOUDANT_URL'] = couchdb_url
        self.couchdb_db = couchdb_db

    @classmethod
    def from_crawler(cls, crawler):
        return cls(
            couchdb_admin=crawler.settings.get('COUCHDB_ADMIN'),
            couchdb_password=crawler.settings.get('COUCHDB_PASSWORD'),
            couchdb_url=crawler.settings.get('COUCHDB_URL'),
            couchdb_db=crawler.settings.get('COUCHDB_DB')
        )

    def open_spider(self, spider):
        self.client = CloudantV1.new_instance()
        server_information = self.client.get_server_information().get_result()
        logging.info(f'CouchDB Server Information: {server_information}')

    def close_spider(self, spider):
        del os.environ['CLOUDANT_AUTH_TYPE']
        del os.environ['CLOUDANT_USERNAME']
        del os.environ['CLOUDANT_PASSWORD']
        del os.environ['CLOUDANT_URL']

    def process_item(self, item, spider):
        if 'deleted' in item and item['deleted']:
            doc_id = item['id']
            try:
                rev = self.client.head_document(
                    db=self.couchdb_db,
                    doc_id=doc_id
                ).get_headers()['ETag'][1:-1]
                response = self.client.delete_document(
                    db=self.couchdb_db,
                    doc_id=doc_id,
                    rev=rev
                ).get_result()
            except ApiException as e:
                logging.info(f'Delete Document Error for: {doc_id} with Error Response: {e.http_response}')
            return item

        changes_rev = item['changes_rev']
        response_json = item['response_json']

        if '_id' not in response_json or '_rev' not in response_json:
            logging.critical(f'Document Error: no _id or no _rev: {item}')
            return item

        if changes_rev != response_json['_rev']:
            doc_id = response_json['_id']
            logging.error(f'Document Error: _rev is not requested: {doc_id}')

            # # npmmirror has different _rev with npmjs
            # logging.critical(f'official_changes_rev: {changes_rev}, mirror_rev: {response_json['_rev']}')
            # response_json['_rev'] = changes_rev 
            # logging.critical(f'official_changes_rev: {changes_rev}, mirror_rev: {response_json['_rev']}')
            return item

        rev = None
        replicate_rev = None
        try:
            response = self.client.get_document(
                db=self.couchdb_db,
                doc_id=response_json['_id']
            ).get_result()
            rev = response['_rev']
            replicate_rev = response['replicate.npmjs.com_rev']
        except ApiException as e:
            pass

        if replicate_rev is not None and changes_rev == replicate_rev:
            return item

        try:
            response_json['replicate.npmjs.com_rev'] = response_json['_rev']
            if rev is None:
                del response_json['_rev']
            else:
                response_json['_rev'] = rev
            response = self.client.post_document(
                db=self.couchdb_db,
                document=response_json,
            ).get_result()
        except ApiException as e:
            response_id = response_json['_id']
            logging.error(f'Post Document Error for: {response_id} with Error Response: {e.http_response.json()}')

        return item
