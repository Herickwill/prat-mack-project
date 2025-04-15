# Entry point da Lambda
import json

def lambda_handler(event, context):
    return {
        'statusCode': 200,
        'body': json.dumps('Testando Gustavo Emilly')
}
