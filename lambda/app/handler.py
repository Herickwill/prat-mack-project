# Entry point da Lambda
import json
import boto3
import os

def lambda_handler(event, context):
    dynamodb = boto3.client('dynamodb')
    print("Medição recebida!")
    dynamodb.put_item(
    TableName=os.environ['DYNAMODB_TABLE'],
    Item={
        'caixa_id': {'S': str(event.get("caixa_id"))},
        'timestamp': {'S': str(event.get("timestamp"))},
        'valor_sensor': {'N': str(event.get("valor_sensor"))}
        }
    )
    return {
        'statusCode': 200,
        'body': json.dumps(event['caixa_id'])
    }
