import json
import os
import pytest
import boto3
from app.handler import lambda_handler

# Fakes dos clientes AWS
class FakeDynamoDB:
    def __init__(self):
        self.items = []
    def put_item(self, TableName, Item):
        # Armazena o item internamente para inspeção, se quiser
        self.items.append((TableName, Item))
        return {'ResponseMetadata': {'HTTPStatusCode': 200}}

class FakeSNS:
    def __init__(self):
        self.published = []
    def publish(self, TopicArn, Message, Subject):
        self.published.append((TopicArn, Message, Subject))
        return {'MessageId': 'fake-id'}

@pytest.fixture(autouse=True)
def setup_env_and_clients(monkeypatch):
    # Configura nome da tabela
    os.environ['DYNAMODB_TABLE'] = 'teste'
    # Instâncias fake
    fake_dynamo = FakeDynamoDB()
    fake_sns   = FakeSNS()

    # Monkeypatch do boto3.client
    def fake_client(service_name, *args, **kwargs):
        if service_name == 'dynamodb':
            return fake_dynamo
        elif service_name == 'sns':
            return fake_sns
        raise RuntimeError(f"Serviço inesperado: {service_name}")

    monkeypatch.setattr(boto3, 'client', fake_client)

    # Disponibiliza os fakes para os testes
    return {'dynamo': fake_dynamo, 'sns': fake_sns}


def test_handler_valor_ok(setup_env_and_clients):
    event = {'caixa_id': 'cx1', 'valor_sensor': 100.0, 'timestamp': 0}
    resp = lambda_handler(event, None)
    assert resp['statusCode'] == 200
    # Ainda não crítico => sem SNS
    assert setup_env_and_clients['sns'].published == []


def test_handler_valor_critico(setup_env_and_clients):
    event = {'caixa_id': 'cx1', 'valor_sensor': 200.0, 'timestamp': 0}
    resp = lambda_handler(event, None)
    assert resp['statusCode'] == 200
    # Valor crítico => SNS deve ter sido chamado
    published = setup_env_and_clients['sns'].published
    assert len(published) == 1

    topic_arn, message, subject = published[0]
    assert 'agua-alertas' in topic_arn
    assert 'Identificamos' in message
    assert 'Problema urgente' in subject
